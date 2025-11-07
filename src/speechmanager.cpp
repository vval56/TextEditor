#include "speechmanager.h"
#include <QAudioDevice>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>

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
    connect(speechProcess, &QProcess::finished, this, &SpeechManager::onSpeechProcessFinished);
    connect(speechProcess, &QProcess::readyReadStandardOutput, this, &SpeechManager::readSpeechOutput);
    connect(speechProcess, &QProcess::readyReadStandardError, this, [this]() {
        const QString err = speechProcess->readAllStandardError();
        if (!err.isEmpty()) {
            handleRecognitionErrorOutput(err);
        }
    });
}

SpeechManager::~SpeechManager()
{
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
    // Используем Python speech_recognition (Google API) для распознавания русского языка
    recognizedText.clear();
    recognitionError.clear();
    recognitionFailed = false;

    const QString pythonPath = resolvePythonExecutable();
    if (pythonPath.isEmpty()) {
        recognitionFailed = true;
        recognitionError = QStringLiteral("Не удалось найти исполняемый файл python3. Установите Python 3 или задайте PYTHON3_PATH.");
        emit errorOccurred(recognitionError);
        return;
    }

    QString moduleError;
    if (!verifyPythonModules(pythonPath, &moduleError)) {
        recognitionFailed = true;
        recognitionError = moduleError.isEmpty()
                                ? QStringLiteral("Python (%1) не нашёл модули speech_recognition или pyaudio. Установите их командой: %2 -m pip install SpeechRecognition pyaudio")
                                      .arg(pythonPath, pythonPath)
                                : moduleError;
        emit errorOccurred(recognitionError);
        return;
    }

    pythonExecutable = pythonPath;
    speechProcess->setProcessEnvironment(buildPythonEnvironment());

#if defined(Q_OS_MACOS) || defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    const char *code =
        "import sys\n"
        "try:\n"
        "    import speech_recognition as sr\n"
        "except Exception as e:\n"
        "    print('ERROR:MODULE', e, file=sys.stderr); sys.exit(2)\n"
        "r = sr.Recognizer()\n"
        "with sr.Microphone() as source:\n"
        "    print('Говорите...')\n"
        "    r.adjust_for_ambient_noise(source, duration=0.5)\n"
        "    audio = r.listen(source, timeout=10, phrase_time_limit=30)\n"
        "try:\n"
        "    text = r.recognize_google(audio, language='ru-RU')\n"
        "    print(text)\n"
        "except Exception as e:\n"
        "    print('ERROR:RECOGNIZE', e, file=sys.stderr); sys.exit(3)\n";

    speechProcess->start(pythonExecutable, QStringList() << "-c" << code);

    if (speechProcess->waitForStarted(1000)) {
        emit listeningStarted();
        emit textRecognized("Слушаю... Говорите сейчас");
    } else {
        recognitionFailed = true;
        recognitionError = QStringLiteral("Не удалось запустить python по пути %1").arg(pythonExecutable);
        emit errorOccurred(QStringLiteral("Не удалось запустить распознавание речи. Проверьте доступность Python по пути %1.").arg(pythonExecutable));
        return;
    }
#else
    emit errorOccurred("Распознавание речи не поддерживается на этой платформе");
    return;
#endif
}

void SpeechManager::stopListening()
{
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
    if (!out.isEmpty()) {
        recognizedText = out.trimmed();
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
