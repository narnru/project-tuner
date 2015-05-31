#include <QCoreApplication>

#include <QDebug>
#include <QByteArray>
#include <QAudioInput.h>
#include<QAudioOutput.h>
#include <QBuffer>
#include <math.h>

#define SampleRate  44100

const int BufferSize = 2*SampleRate*sizeof(short); // 1 second = SampleRate * 16bit
QAudioDeviceInfo m_Inputdevice = QAudioDeviceInfo::defaultInputDevice();
QAudioDeviceInfo m_Outputdevice = QAudioDeviceInfo::defaultOutputDevice();
QAudioFormat m_format;
QAudioInput *m_audioInput = 0;
QAudioOutput *m_audioOutput = 0;
QIODevice *m_input = 0;
QIODevice *m_output = 0;
QByteArray m_buffer;


const double TwoPi = 6.283185307179586;

void FFTAnalysis(double *AVal, double *FTvl, int Nvl, int Nft) {
  int i, j, n, m, Mmax, Istp;
  double Tmpr, Tmpi, Wtmp, Theta;
  double Wpr, Wpi, Wr, Wi;
  double *Tmvl;

  n = Nvl * 2;
  Tmvl = (double*)malloc(sizeof(double)*n);

  for (i = 0; i < n; i+=2) {
   Tmvl[i] = 0;
   Tmvl[i+1] = AVal[i/2];
  }

  i = 1; j = 1;
  while (i < n) {
    if (j > i) {
      Tmpr = Tmvl[i];
      Tmvl[i] = Tmvl[j];
      Tmvl[j] = Tmpr;
      Tmpr = Tmvl[i+1];
      Tmvl[i+1] = Tmvl[j+1];
      Tmvl[j+1] = Tmpr;
    }
    i = i + 2;
    m = Nvl;
    while ((m >= 2) && (j > m)) {
      j = j - m;
      m = m >> 1;
    }
    j = j + m;
  }

  Mmax = 2;
  while (n > Mmax) {
    Theta = -TwoPi / Mmax;
    Wpi = sin(Theta);
    Wtmp = sin(Theta / 2);
    Wpr = Wtmp * Wtmp * 2;
    Istp = Mmax * 2;
    Wr = 1;
    Wi = 0;
    m = 1;

    while (m < Mmax) {
      i = m;
      m = m + 2;
      Tmpr = Wr;
      Tmpi = Wi;
      Wr = Wr - Tmpr * Wpr - Tmpi * Wpi;
      Wi = Wi + Tmpr * Wpi - Tmpi * Wpr;

      while (i < n) {
        j = i + Mmax;
        Tmpr = Wr * Tmvl[j] - Wi * Tmvl[j-1];
        Tmpi = Wi * Tmvl[j] + Wr * Tmvl[j-1];

        Tmvl[j] = Tmvl[i] - Tmpr;
        Tmvl[j-1] = Tmvl[i-1] - Tmpi;
        Tmvl[i] = Tmvl[i] + Tmpr;
        Tmvl[i-1] = Tmvl[i-1] + Tmpi;
        i = i + Istp;
      }
    }

    Mmax = Istp;
  }

  for (i = 0; i < Nft; i++) {
    j = i * 2;
    FTvl[Nft - i - 1] = sqrt(pow(Tmvl[j],2) + pow(Tmvl[j+1],2));
  }

  free (Tmvl);
}



void InitializeAudio()
{
    m_format.setSampleRate(SampleRate); //set frequency to 44100
    m_format.setChannelCount(1); //set channels to mono
    m_format.setSampleSize(16); //set sample sze to 16 bit
    m_format.setSampleType(QAudioFormat::SignedInt ); //Sample type as usigned integer sample UnSignedInt
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

double  ReadData()
{
    //Return if audio input is null
    if(!m_audioInput)
        return 0;

    //Check the number of samples in input buffer
    qint64 len = m_audioInput->bytesReady();

    if(len >= BufferSize/2)
    {
        printf("bytesReady = %lld \n", len);
         m_buffer.resize(len);

        //Read sound samples from input device to buffer
        qint64 ret = m_input->read(m_buffer.data(), len);

        printf("bytesReady = %lld, byte_read = %lld\n", len, ret);

        //Assign sound samples to short array
        short* data = (short*)m_buffer.data();

        m_output->write((char*)data, ret); // play sound!!

        int word_read = ret/sizeof(short);

        double* result = (double*)malloc(sizeof(double)*len);
        int i;
       for (i=0; i < word_read; i++)
        {
            data[i] = data[i]; // можно менять громкость с микро))
//            if (data[i]/35000.0 < 0.105)
//            {
//                data[i] = 0;
//            }
       }
        qint64 len2 = 256;
        while (2*len2 <= word_read)
        {
            len2 = 2*len2;
        }
        printf("len2 = %lld\n", len2);
        double* data2 = (double*)malloc(sizeof(double)*len2);
       for (i=0; i < len2; i++)
        {
            data2[i] = data[i]/35000.0; // cos(TwoPi*i/44.1); а прост) честно я не знаю нафига я это сделал. Мне маленькие числа нравятся больше)))
       }

        FFTAnalysis(data2, result, len2, len2);

        FILE *f = fopen("spectrum.txt", "w");
        double max = 0;
        double maxi = -1;
          for(i=0; i<len2/2; i++)
          {
            fprintf(f, "%d  %lf  %lf\n", data[i], data2[i], result[i]);
            if(max < result[i])
            {
                max = result[i];
                maxi = i;
            }
          }
          fclose(f);
           printf("%lf %lf \n", max, maxi*SampleRate/len2);

        free(data2);
        free(result);

        m_audioOutput->stop();
        m_audioInput->stop();

        m_output= m_audioOutput->start();
        m_input = m_audioInput->start();

        return max;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
   // m_buffer.resize(BufferSize);
    InitializeAudio();

    m_audioInput->setBufferSize(BufferSize);
    m_audioOutput->setBufferSize(BufferSize);

    printf("BufferSize = %d \n", BufferSize);

    //Audio output device
    m_output= m_audioOutput->start();
     //Audio input device
    m_input = m_audioInput->start();

    while (1==1)
    {
        double ret = ReadData();
//        if(ret > 0.04)
//            return 0;
    }
    return a.exec();
}
