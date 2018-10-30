#pragma once

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QTextStream>

#include <exception>

using std::runtime_error;

namespace DOGIToys {

class DOGIFile {
private:
  QTextStream stream;
  QFile file{};

public:
  QString line{};

  DOGIFile() = delete;

  DOGIFile(const QString &file_name) {
    if (!QFileInfo::exists(file_name))
      throw runtime_error("File does not exist " + file_name.toStdString());

    file.setFileName(file_name);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      throw runtime_error("Can't open file: " + file.fileName().toStdString());

    stream.setDevice(&file);
  }

  ~DOGIFile() { file.close(); }

  bool readLineInto(QString *external = nullptr) {
    if (external)
      return stream.readLineInto(external);
    else
      return stream.readLineInto(&line);
  }

  void skipLines(const QChar &ch = '#', QString *external = nullptr) {
    if (external) {
      while (readLineInto(external) and external->startsWith(ch))
        continue;
    } else {
      while (readLineInto() and this->line.startsWith(ch))
        continue;
    }
  }

  void passLines(int count = 0) {
    for (int i = 0; i < count; ++i)
      stream.readLine();
  }
};
} // namespace DOGIToys
