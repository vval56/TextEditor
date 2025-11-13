#include "speechmanager.h"
#include <QAudioDevice>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDataStream>
#include <QBuffer>
#include <QDir>
#include <QThread>
#include <QElapsedTimer>

SpeechManager::SpeechManager(QObject *parent)
    : QObject(parent)
{
    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    speechProcess = new QProcess(this);

    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.8f);

    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &SpeechManager::onMediaStatusChanged);
    connect(mediaPlayer, &QMediaPlayer::errorOccurred, this, &SpeechManager::onPlayerError);
    // Сливаем stderr в stdout, чтобы читать всё единым потоком (упростит диагностику)
    speechProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(speechProcess, &QProcess::finished, this, &SpeechManager::onSpeechProcessFinished);
    connect(speechProcess, &QProcess::readyReadStandardOutput, this, &SpeechManager::readSpeechOutput);
    connect(speechProcess, &QProcess::readyReadStandardError, this, [this]() {
        const QString err = speechProcess->readAllStandardError();
        if (!err.isEmpty()) {
            handleRecognitionErrorOutput(err);
        }
    });
}

void SpeechManager::startQtRecording(quint32 msec)
{
    if (isRecording) return;
    QAudioFormat fmt;
    fmt.setSampleRate(16000);
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);

    audioSource = new QAudioSource(fmt, this);
    tempWavPath = makeTempWavPath();
    if (wavFile) { delete wavFile; wavFile = nullptr; }
    wavFile = new QFile(tempWavPath, this);
    if (!wavFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        emit errorOccurred(QStringLiteral("Не удалось открыть файл для записи: ") + tempWavPath);
        delete audioSource; audioSource = nullptr; delete wavFile; wavFile = nullptr;
        return;
    }
    wavFile->write(makeWavHeader(fmt.sampleRate(), fmt.channelCount(), 16, 0));
    recordedBytes = 0;
    audioDevice = audioSource->start();
    if (!audioDevice) {
        emit errorOccurred(QStringLiteral("Не удалось запустить запись аудио"));
        wavFile->close(); delete wavFile; wavFile = nullptr; delete audioSource; audioSource = nullptr;
        return;
    }
    if (!pollTimer) { pollTimer = new QTimer(this); }
    connect(pollTimer, &QTimer::timeout, this, [this]() {
        if (!audioDevice || !wavFile) return;
        QByteArray data = audioDevice->readAll();
        if (!data.isEmpty()) {
            wavFile->write(data);
            recordedBytes += data.size();
        }
    });
    pollTimer->start(30);

    if (!recordTimeout) { recordTimeout = new QTimer(this); recordTimeout->setSingleShot(true); }
    connect(recordTimeout, &QTimer::timeout, this, [this, fmt]() {
        stopQtRecording(false);
        finalizeWavFile(fmt.sampleRate(), fmt.channelCount(), 16);

        // Подождем, пока файл гарантированно получит корректный заголовок RIFF/WAVE
        bool wavOk = false;
        for (int i = 0; i < 10; ++i) {
            QFile checkFile(tempWavPath);
            if (checkFile.open(QIODevice::ReadOnly)) {
                QByteArray hdr = checkFile.read(12);
                checkFile.close();
                if (hdr.size() >= 12 && hdr.mid(0, 4) == "RIFF" && hdr.mid(8, 4) == "WAVE") {
                    wavOk = true;
                    break;
                }
            }
            QThread::msleep(50);
        }
        if (!wavOk) {
            emit errorOccurred(QStringLiteral("Некорректный WAV: нет заголовка RIFF/WAVE"));
            return;
        }

        const QString pythonPath = resolvePythonExecutable();
        if (pythonPath.isEmpty()) { emit errorOccurred(QStringLiteral("Не найден python3")); return; }
        pythonExecutable = pythonPath;

        QProcessEnvironment env = buildPythonEnvironment();
        QString voskModelPath = QString::fromLocal8Bit(qgetenv("VOSK_MODEL_PATH")).trimmed();
        if (!voskModelPath.isEmpty() && voskModelPath.startsWith("~")) {
            voskModelPath.replace(0, 1, QDir::homePath());
        }
        if (!voskModelPath.isEmpty()) env.insert("VOSK_MODEL_PATH", voskModelPath);
        env.insert("VOSK_LOG_LEVEL", "-1");
        speechProcess->setProcessEnvironment(env);
        speechProcess->setProcessChannelMode(QProcess::MergedChannels);

        QByteArray code = QByteArray()
            + "import sys, os, json\n"
              "import wave\n"
              "from vosk import Model, KaldiRecognizer, SetLogLevel\n"
              "model_path = os.environ.get('VOSK_MODEL_PATH','').strip()\n"
              "try:\n"
              "    SetLogLevel(-1)\n"
              "except Exception:\n"
              "    pass\n"
              "if not model_path or not os.path.isdir(model_path):\n"
              "    print('ERROR:VOSK_MODEL', model_path, file=sys.stderr); sys.exit(2)\n"
              "wf = wave.open(r'" + tempWavPath.toUtf8() + "', 'rb')\n"
              "if wf.getnchannels()!=1 or wf.getsampwidth()!=2:\n"
              "    print('ERROR:WAV_FORMAT', file=sys.stderr); sys.exit(3)\n"
              "model = Model(model_path)\n"
              "rec = KaldiRecognizer(model, wf.getframerate())\n"
              "txt=''\n"
              "while True:\n"
              "    data = wf.readframes(4000)\n"
              "    if len(data)==0: break\n"
              "    if rec.AcceptWaveform(data):\n"
              "        pass\n"
              "res = json.loads(rec.FinalResult())\n"
              "txt = res.get('text','').strip()\n"
              "if not txt:\n"
              "    try:\n"
              "        import speech_recognition as sr\n"
              "        r = sr.Recognizer()\n"
              "        with sr.AudioFile(r'" + tempWavPath.toUtf8() + "') as source:\n"
              "            audio = r.record(source)\n"
              "        try:\n"
              "            txt = r.recognize_google(audio, language='ru-RU')\n"
              "        except Exception:\n"
              "            txt = ''\n"
              "    except Exception as e:\n"
              "        pass\n"
              "print(txt, flush=True)\n";

        if (speechProcess->state() != QProcess::NotRunning) {
            emit errorOccurred(QStringLiteral("Процесс распознавания уже запущен"));
            return;
        }
        speechProcess->start(pythonExecutable, QStringList() << "-u" << "-c" << QString::fromUtf8(code));
        if (speechProcess->waitForStarted(1000)) {
            emit listeningStarted();
        } else {
            emit errorOccurred(QStringLiteral("Не удалось запустить python для распознавания"));
        }
    });

    recordTimeout->start(msec);
    emit listeningStarted();
    emit textRecognized(QStringLiteral("Слушаю... Говорите сейчас"));
    isRecording = true;
}

void SpeechManager::stopQtRecording(bool cancel)
{
    if (!isRecording) return;
    if (pollTimer) { pollTimer->stop(); pollTimer->disconnect(this); }
    if (recordTimeout) { recordTimeout->stop(); recordTimeout->disconnect(this); }
    if (audioSource) { audioSource->stop(); }
    if (audioDevice) { audioDevice->close(); }
    if (wavFile) { wavFile->flush(); wavFile->close(); }
    isRecording = false;
}

void SpeechManager::finalizeWavFile(quint32 sampleRate, quint16 channels, quint16 bitsPerSample)
{
    if (!wavFile) return;
    QFile f(wavFile->fileName());
    if (!f.open(QIODevice::ReadWrite)) return;
    quint32 dataSize = f.size() > 44 ? static_cast<quint32>(f.size() - 44) : 0;
    f.seek(0);
    f.write(makeWavHeader(sampleRate, channels, bitsPerSample, dataSize));
    f.close();
}

QByteArray SpeechManager::makeWavHeader(quint32 sampleRate, quint16 channels, quint16 bitsPerSample, quint32 dataSize) const
{
    QByteArray header;
    QBuffer buf(&header);
    buf.open(QIODevice::WriteOnly);
    QDataStream s(&buf);
    s.setByteOrder(QDataStream::LittleEndian);

    auto write4 = [&](const char *tag){ buf.write(tag, 4); };

    write4("RIFF");
    quint32 chunkSize = 36 + dataSize; s << chunkSize;
    write4("WAVE");
    write4("fmt ");
    quint32 subChunk1Size = 16; s << subChunk1Size;
    quint16 audioFormat = 1; s << audioFormat;
    s << channels;
    s << sampleRate;
    quint32 byteRate = sampleRate * channels * bitsPerSample / 8; s << byteRate;
    quint16 blockAlign = channels * bitsPerSample / 8; s << blockAlign;
    s << bitsPerSample;
    write4("data");
    s << dataSize;
    buf.close();
    return header;
}

QString SpeechManager::makeTempWavPath() const
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (base.isEmpty()) base = QDir::tempPath();
    return base + QDir::separator() + "qt_speech_record.wav";
}

SpeechManager::~SpeechManager()
{
    if (speechProcess) {
        if (speechProcess->state() == QProcess::Running) {
            speechProcess->terminate();
            if (!speechProcess->waitForFinished(2000)) {
                speechProcess->kill();
                speechProcess->waitForFinished(1000);
            }
        }
    }
    delete mediaPlayer;
    delete audioOutput;
    delete speechProcess;
}

void SpeechManager::speakText(const QString &text)
{
    if (text.isEmpty()) {
        emit errorOccurred("Текст для озвучивания пуст");
        return;
    }

    speakWithSystem(text);
}

void SpeechManager::speakWithSystem(const QString &text)
{
    QProcess *process = new QProcess(this);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitCode)
                Q_UNUSED(exitStatus)
                process->deleteLater();
                emit speechFinished();
            });

    emit speechStarted();
    process->start("say", QStringList() << text);
}

void SpeechManager::stopSpeaking()
{
    QProcess::execute("killall", QStringList() << "say");
    emit speechFinished();
}

void SpeechManager::startListening()
{
    if (speechProcess->state() == QProcess::Running) {
        emit errorOccurred(QStringLiteral("Прослушивание уже запущено"));
        return;
    }
    recognizedText.clear();
    recognitionError.clear();
    recognitionFailed = false;

    const QString pythonPath = resolvePythonExecutable();
    if (pythonPath.isEmpty()) { emit errorOccurred(QStringLiteral("Не найден python3")); return; }
    pythonExecutable = pythonPath;

    QProcessEnvironment env = buildPythonEnvironment();
    QString voskModelPath = QString::fromLocal8Bit(qgetenv("VOSK_MODEL_PATH")).trimmed();
    if (!voskModelPath.isEmpty() && voskModelPath.startsWith("~")) {
        voskModelPath.replace(0, 1, QDir::homePath());
    }
    if (!voskModelPath.isEmpty()) env.insert("VOSK_MODEL_PATH", voskModelPath);
    env.insert("VOSK_LOG_LEVEL", "-1");
    speechProcess->setProcessEnvironment(env);
    speechProcess->setProcessChannelMode(QProcess::MergedChannels);

    // Python: запись с микрофона через sounddevice ~7 сек и распознавание через Vosk; при пустом результате — фоллбек Google
    QByteArray code = QByteArray()
        + "import sys, os, json\n"
          "model_path = os.environ.get('VOSK_MODEL_PATH','').strip()\n"
          "try:\n"
          "    from vosk import Model, KaldiRecognizer, SetLogLevel\n"
          "    import sounddevice as sd\n"
          "    import numpy as np\n"
          "except Exception as e:\n"
          "    print('ERROR:MODULE', e, file=sys.stderr); sys.exit(1)\n"
          "if not model_path or not os.path.isdir(model_path):\n"
          "    print('ERROR:VOSK_MODEL', model_path, file=sys.stderr); sys.exit(2)\n"
          "try:\n"
          "    SetLogLevel(-1)\n"
          "except Exception:\n"
          "    pass\n"
          "duration = 7\n"
          "try:\n"
          "    default_sr = int(sd.query_devices(kind='input')['default_samplerate'])\n"
          "except Exception:\n"
          "    default_sr = 16000\n"
          "samplerate = 16000 if default_sr <= 0 else int(default_sr)\n"
          "channels = 1\n"
          "model = Model(model_path)\n"
          "rec = KaldiRecognizer(model, samplerate)\n"
          "def callback(indata, frames, time, status):\n"
          "    if status: pass\n"
          "    try:\n"
          "        data = indata.copy().astype('int16').tobytes()\n"
          "    except Exception:\n"
          "        data = indata.tobytes()\n"
          "    rec.AcceptWaveform(data)\n"
          "try:\n"
          "    with sd.InputStream(samplerate=samplerate, channels=channels, dtype='int16', callback=callback):\n"
          "        sd.sleep(duration * 1000)\n"
          "except Exception as e:\n"
          "    print('ERROR:MIC', e, file=sys.stderr); sys.exit(4)\n"
          "res = json.loads(rec.FinalResult())\n"
          "txt = res.get('text','').strip()\n"
          "if not txt:\n"
          "    try:\n"
          "        import speech_recognition as sr\n"
          "        r = sr.Recognizer()\n"
          "        with sr.Microphone(sample_rate=samplerate) as source:\n"
          "            audio = r.record(source, duration=duration)\n"
          "        try:\n"
          "            txt = r.recognize_google(audio, language='ru-RU')\n"
          "        except Exception:\n"
          "            txt = ''\n"
          "    except Exception:\n"
          "        pass\n"
          "print(txt, flush=True)\n";

    speechProcess->start(pythonExecutable, QStringList() << "-u" << "-c" << QString::fromUtf8(code));
    if (speechProcess->waitForStarted(1000)) {
        emit listeningStarted();
        emit textRecognized(QStringLiteral("Слушаю... Говорите сейчас"));
    } else {
        emit errorOccurred(QStringLiteral("Не удалось запустить python для распознавания"));
    }
}

void SpeechManager::stopListening()
{
    stopQtRecording(true);
    if (speechProcess->state() == QProcess::Running) {
        speechProcess->terminate();
        speechProcess->waitForFinished();
    }
    emit listeningFinished();
}

bool SpeechManager::isListening() const
{
    return speechProcess->state() == QProcess::Running;
}

bool SpeechManager::isSpeaking() const
{
    QProcess process;
    process.start("pgrep", QStringList() << "say");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    return !output.trimmed().isEmpty();
}

void SpeechManager::setSpeechVolume(float volume)
{
    if (audioOutput) {
        audioOutput->setVolume(qBound(0.0f, volume, 1.0f));
    }
}

void SpeechManager::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia) {
        emit speechStarted();
    } else if (status == QMediaPlayer::EndOfMedia) {
        emit speechFinished();
    }
}

void SpeechManager::onPlayerError(QMediaPlayer::Error error, const QString &errorString)
{
    Q_UNUSED(error)
    emit errorOccurred("Ошибка медиаплеера: " + errorString);
}

void SpeechManager::onSpeechProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!recognizedText.isEmpty()) {
        emit textRecognized(recognizedText);
    } else if (recognitionFailed) {
        emit errorOccurred(recognitionError.isEmpty()
                               ? QStringLiteral("Распознавание речи завершилось с ошибкой")
                               : recognitionError);
    } else if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        emit errorOccurred("Распознавание речи завершилось преждевременно. Попробуйте снова.");
    }
    emit listeningFinished();
}

void SpeechManager::readSpeechOutput()
{
    QString out = speechProcess->readAllStandardOutput();
    if (out.isEmpty()) return;
    // Разбиваем по строкам и обрабатываем каждую
    const QStringList lines = out.split('\n', Qt::SkipEmptyParts);
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;
        if (line.startsWith(QStringLiteral("Говорите"))) continue; // сервисное
        if (line.startsWith(QStringLiteral("INFO:"))) continue;    // диагностическое
        if (line.startsWith(QStringLiteral("ERROR:")) ||
            line.startsWith(QStringLiteral("Traceback")) ||
            line.contains(QStringLiteral("wave.Error"))) {           // ошибка из Python
            handleRecognitionErrorOutput(line);
            continue;
        }
        // Фильтруем детальные логи Vosk/Kaldi
        if (line.startsWith(QStringLiteral("LOG ")) ||
            line.startsWith(QStringLiteral("LOG(")) ||
            line.contains(QStringLiteral("(VoskAPI:")) ||
            line.contains(QStringLiteral("ReadDataFiles")) ||
            line.contains(QStringLiteral("ComputeDerivedVars"))) {
            continue;
        }
        recognizedText = line;
        emit textRecognized(recognizedText);
    }
}

void SpeechManager::handleRecognitionErrorOutput(const QString &errorOutput)
{
    recognitionFailed = true;
    if (errorOutput.contains("ERROR:MODULE")) {
        recognitionError = QStringLiteral("Не найден модуль speech_recognition или pyaudio. Установите команды: pip3 install SpeechRecognition pyaudio");
    } else if (errorOutput.contains("ERROR:RECOGNIZE")) {
        recognitionError = QStringLiteral("Не удалось распознать речь. Убедитесь, что микрофон подключен, и попробуйте снова.");
    } else {
        recognitionError = errorOutput.trimmed();
    }
}

QString SpeechManager::resolvePythonExecutable() const
{
    QStringList candidates;
    const QString envCandidate = QString::fromLocal8Bit(qgetenv("PYTHON3_PATH"));
    if (!envCandidate.trimmed().isEmpty()) {
        candidates << envCandidate.trimmed();
    }

    const QString pathCandidate = QStandardPaths::findExecutable(QStringLiteral("python3"));
    if (!pathCandidate.isEmpty()) {
        candidates << pathCandidate;
    }

    candidates << QStringLiteral("/Library/Frameworks/Python.framework/Versions/Current/bin/python3")
               << QStringLiteral("/Library/Frameworks/Python.framework/Versions/3.14/bin/python3")
               << QStringLiteral("/opt/homebrew/bin/python3")
               << QStringLiteral("/usr/local/bin/python3")
               << QStringLiteral("/usr/bin/python3");

    QStringList unique;
    for (const QString &candidate : std::as_const(candidates)) {
        if (candidate.isEmpty()) {
            continue;
        }
        QFileInfo info(candidate);
        QString normalized = info.exists() ? info.absoluteFilePath() : candidate;
        if (!unique.contains(normalized)) {
            unique << normalized;
        }
    }

    for (const QString &candidate : std::as_const(unique)) {
        QFileInfo info(candidate);
        if (info.exists() && info.isExecutable()) {
            return info.absoluteFilePath();
        }
    }

    return QString();
}

QProcessEnvironment SpeechManager::buildPythonEnvironment() const
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString path = env.value(QStringLiteral("PATH"));

    const QStringList extraPaths = {
        QStringLiteral("/opt/homebrew/bin"),
        QStringLiteral("/usr/local/bin"),
        QStringLiteral("/Library/Frameworks/Python.framework/Versions/Current/bin"),
        QStringLiteral("/Library/Frameworks/Python.framework/Versions/3.14/bin")
    };

    const QString separator = QString(QDir::listSeparator());
    QStringList segments = path.split(QDir::listSeparator(), Qt::SkipEmptyParts);
    for (const QString &extra : extraPaths) {
        if (!segments.contains(extra)) {
            segments.append(extra);
        }
    }

    env.insert(QStringLiteral("PATH"), segments.join(separator));
    return env;
}

bool SpeechManager::verifyPythonModules(const QString &pythonPath, QString *errorMessage) const
{
    QProcess checkProcess;
    checkProcess.setProcessEnvironment(buildPythonEnvironment());

    const char *checkCode =
        "import importlib, sys\n"
        "missing = []\n"
        "for name in ('speech_recognition', 'pyaudio'):\n"
        "    try:\n"
        "        importlib.import_module(name)\n"
        "    except Exception as e:\n"
        "        missing.append(f'{name}: {e}')\n"
        "if missing:\n"
        "    sys.stderr.write('ERROR:MODULE ' + '; '.join(missing))\n"
        "    sys.exit(1)\n";

    checkProcess.start(pythonPath, QStringList() << "-c" << checkCode);
    if (!checkProcess.waitForStarted(1000)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Не удалось запустить %1 для проверки модулей.").arg(pythonPath);
        }
        return false;
    }

    checkProcess.waitForFinished();

    if (checkProcess.exitStatus() != QProcess::NormalExit || checkProcess.exitCode() != 0) {
        QString stderrOutput = QString::fromUtf8(checkProcess.readAllStandardError()).trimmed();
        if (stderrOutput.isEmpty()) {
            stderrOutput = QStringLiteral("Python (%1) не загрузил модули SpeechRecognition/PyAudio.").arg(pythonPath);
        }
        if (errorMessage) {
            *errorMessage = stderrOutput;
        }
        return false;
    }

    return true;
}
