#ifndef SPEECHMANAGER_H
#define SPEECHMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QProcess>
#include "idocument.h"

class SpeechManager : public QObject
{
    Q_OBJECT

public:
    explicit SpeechManager(QObject *parent = nullptr);
    ~SpeechManager() override;

    void speakText(const QString &text);
    void speakFromDocument(const IDocument *document);
    void stopSpeaking();
    void setSpeechVolume(float volume);

signals:
    void speechStarted();
    void speechFinished();
    void errorOccurred(const QString &error);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlayerError(QMediaPlayer::Error error, const QString &errorString);

private:
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;

    void speakWithSystem(const QString &text);
};

#endif
