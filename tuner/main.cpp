#include <QCoreApplication>

#include <QDebug>
#include <QByteArray>
#include <QAudioInput.h>
#include<QAudioOutput.h>
#include <QBuffer>
#include "Complex.h"

#define SampleRate  44100

const int BufferSize = SampleRate*sizeof(short); // 1 second = SampleRate * 16bit
QAudioDeviceInfo m_Inputdevice = QAudioDeviceInfo::defaultInputDevice();
QAudioDeviceInfo m_Outputdevice = QAudioDeviceInfo::defaultOutputDevice();
QAudioFormat m_format;
QAudioInput *m_audioInput = 0;
QAudioOutput *m_audioOutput = 0;
QIODevice *m_input = 0;
QIODevice *m_output = 0;
QByteArray m_buffer;

void InitializeAudio()
{
    m_format.setSampleRate(SampleRate); //set frequency to 44100
    m_format.setChannelCount(1); //set channels to mono
    m_format.setSampleSize(16); //set sample sze to 16 bit
    m_format.setSampleType(QAudioFormat::UnSignedInt ); //Sample type as usigned integer sample
    m_format.setByteOrder(QAudioFormat::LittleEndian); //Byte order
    m_format.setCodec("audio/pcm"); //set codec as simple audio/pcm

    QAudioDeviceInfo infoIn(QAudioDeviceInfo::defaultInputDevice());
    if (!infoIn.isFormatSupported(m_format))
    {
        //Default format not supported - trying to use nearest
        m_format = infoIn.nearestFormat(m_format);
    }

    QAudioDeviceInfo infoOut(QAudioDeviceInfo::defaultOutputDevice());

    if (!infoOut.isFormatSupported(m_format))
    {
       //Default format not supported - trying to use nearest
        m_format = infoOut.nearestFormat(m_format);
    }

    m_audioInput = new QAudioInput(m_Inputdevice, m_format);
    m_audioOutput = new QAudioOutput(m_Outputdevice, m_format);
}

qint64  ReadData()
{
    //Return if audio input is null
    if(!m_audioInput)
        return 0;

    //Check the number of samples in input buffer
    qint64 len = m_audioInput->bytesReady();

    //Read sound samples from input device to buffer
    qint64 ret = m_input->read(m_buffer.data(), len);
    if(ret > 0)
    {
        //Assign sound samples to short array
        short* data = (short*)m_buffer.data();
        //printf("len = %lld, ret = %lld\n", len, ret);
        int i;
        for (i=0;i<len;i++)
        {
            data[i] = 1*data[i]; // можно менять громкость с микро))
        }
         //write modified sond sample to outputdevice for playback audio
          m_output->write((char*)data, len);
          return ret;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    m_buffer.resize(BufferSize);
    InitializeAudio();

    m_audioInput->setBufferSize(BufferSize);
    m_audioOutput->setBufferSize(BufferSize);

    //Audio output device
    m_output= m_audioOutput->start();
     //Audio input device
    m_input = m_audioInput->start();
    char ch = 0;
    while (1==1)
    {
        qint64 len = ReadData();
    }
    return a.exec();
}
