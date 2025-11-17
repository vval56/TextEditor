#include "speechmanager.h"

SpeechManager::SpeechManager(QObject *parent)
    : QObject(parent)
{
    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);

    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.8f);

    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &SpeechManager::onMediaStatusChanged);
    connect(mediaPlayer, &QMediaPlayer::errorOccurred, this, &SpeechManager::onPlayerError);
}

SpeechManager::~SpeechManager()
{
    delete mediaPlayer;
    delete audioOutput;
}

void SpeechManager::speakText(const QString &text)
{
    if (text.isEmpty()) {
        emit errorOccurred("Текст для озвучивания пуст");
        return;
    }

    speakWithSystem(text);
}

void SpeechManager::speakFromDocument(const IDocument *document)
{
    if (!document) {
        emit errorOccurred(QStringLiteral("Документ для озвучивания не задан"));
        return;
    }

    QString text = document->getSelectedText();
    if (text.isEmpty()) {
        text = document->getAllText();
    }

    speakText(text);
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
