#ifndef SPEECHMANAGER_H
#define SPEECHMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioSource>
#include <QAudioFormat>
#include <QFile>
#include <QTimer>
#include <QProcess>
#include <QDebug>
#include <QProcessEnvironment>

class SpeechManager : public QObject
{
    Q_OBJECT

public:
    explicit SpeechManager(QObject *parent = nullptr);
    ~SpeechManager() override;

    // Синтез речи (озвучивание текста)
    void speakText(const QString &text);
    void stopSpeaking();
    void setSpeechVolume(float volume);

    // Распознавание речи
    void startListening();
    void stopListening();
    bool isListening() const;

    bool isSpeaking() const;

signals:
    void textRecognized(const QString &text);
    void speechStarted();
    void speechFinished();
    void listeningStarted();
    void listeningFinished();
    void errorOccurred(const QString &error);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlayerError(QMediaPlayer::Error error, const QString &errorString);
    void onSpeechProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void readSpeechOutput();

private:
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    QProcess *speechProcess;
    QString recognizedText;
    QString recognitionError;
    bool recognitionFailed = false;
    QString pythonExecutable;

    void speakWithSystem(const QString &text);
    void handleRecognitionErrorOutput(const QString &errorOutput);
    QString resolvePythonExecutable() const;
    QProcessEnvironment buildPythonEnvironment() const;
    bool verifyPythonModules(const QString &pythonPath, QString *errorMessage) const;

    QAudioSource *audioSource = nullptr;
    QIODevice *audioDevice = nullptr;
    QFile *wavFile = nullptr;
    QTimer *pollTimer = nullptr;
    QTimer *recordTimeout = nullptr;
    QString tempWavPath;
    bool isRecording = false;
    quint32 recordedBytes = 0;

    void startQtRecording(quint32 msec);
    void stopQtRecording(bool cancel);
    void finalizeWavFile(quint32 sampleRate, quint16 channels, quint16 bitsPerSample);
    QByteArray makeWavHeader(quint32 sampleRate, quint16 channels, quint16 bitsPerSample, quint32 dataSize) const;
    QString makeTempWavPath() const;
};

#endif // SPEECHMANAGER_H
