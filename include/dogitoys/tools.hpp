#pragma once

#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>

//#include <exception>
#include <optional>
#include <stdexcept>
#include <string>

#include <agizmo/files.hpp>

using runerror = std::runtime_error;

namespace DOGIToys {

// inline static vec_str gff3_header{"SEQID",  "SOURCE", "TYPE",
//                                  "START",  "END",    "SCORE",
//                                  "STRAND", "PHASE",  "ATTRIBUTES"};

// inline static constexpr char gff3_escape[]{"\t\n\r%;=&,"};

// inline string gff3_str_escape(const string &input) {
//  sstream message;
//  size_t last{0}, pos{input.find_first_of(gff3_escape)};

//  if (pos == string::npos)
//    return input;

//  do {
//    message << input.substr(last, pos - last);
//    if (pos == string::npos)
//      break;
//    message << "%" << std::hex << std::uppercase << int(input.at(pos));
//    last = pos + 1;
//    pos = input.find_first_of(gff3_escape, last);
//  } while (true);

//  return message.str();
//}

// inline QString gff3_str_clean(QString input) {
//  int pos = input.indexOf('%');
//  while (pos != -1) {
//    auto ele = char(input.mid(pos + 1, 2).toInt(Q_NULLPTR, 16));
//    input.replace(pos, 3, QString(ele));
//    pos = input.indexOf('%', pos + 1);
//  }
//  return input;
//}

[[noreturn]] inline void throw_runerror(const char *message) {
  throw runerror{message};
}

[[noreturn]] inline void throw_runerror(const QString &message) {
  throw runerror{message.toStdString()};
}

[[noreturn]] inline void throw_runerror(const std::string &message) {
  throw runerror{message};
}
using namespace AGizmo::Files;
class QFileReader {
 private:
  FileReader reader;
  QString line{};

 public:
  QFileReader() = delete;
  QFileReader(const QString &file_name) : reader{file_name.toStdString()} {}

  ~QFileReader() { reader.close(); }

  bool good() const { return reader.good(); }

  bool readLine(const QString &skip = {}) {
    if (reader.readLine(skip.toStdString())) {
      line = QString::fromStdString(reader.getLine());
      return true;
    } else
      return false;
  }

  bool readLine(int skip) {
    if (reader.readLine(skip)) {
      line = QString::fromStdString(reader.getLine());
      return true;
    } else
      return false;
  }

  bool setLineToMatch(const QString &match) {
    do {
      readLine();
      if (line == match) return true;
    } while (good());

    return false;
  }

  std::optional<QString> operator()(const QString &skip = {}) {
    if (readLine(skip))
      return line;
    else
      return nullopt;
  }

  std::optional<QString> operator()(int skip) {
    if (readLine(skip))
      return line;
    else
      return nullopt;
  }
};

inline QPair<QString, QString> get_field_value(const QString &source,
                                               QChar separator,
                                               const int spaces = 0) {
  const auto sep_index = source.indexOf(separator);
  QString field = source.left(sep_index);
  QString value = source.mid(sep_index + 1 + spaces);

  return {field, value};
}

inline QString extractID(const QString &id, const QChar separator) {
  auto sep = id.indexOf(separator) + 1;
  auto last = id.indexOf(' ', sep + 1) - sep;
  return id.mid(sep, last);
}

inline int extractIntID(const QString &id, const QChar separator) {
  return extractID(id, separator).toInt();
}

}  // namespace DOGIToys
