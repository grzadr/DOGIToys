#pragma once

#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>

#include <exception>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using sstream = std::stringstream;
using vec_str = std::vector<string>;

using runerror = std::runtime_error;

namespace DOGIToys::DOGITools {

inline static vec_str gff3_header{"SEQID",  "SOURCE", "TYPE",
                                  "START",  "END",    "SCORE",
                                  "STRAND", "PHASE",  "ATTRIBUTES"};

inline static constexpr char gff3_escape[]{"\t\n\r%;=&,"};

inline string gff3_str_escape(const string &input) {
  sstream message;
  size_t last{0}, pos{input.find_first_of(gff3_escape)};

  if (pos == string::npos)
    return input;

  do {
    message << input.substr(last, pos - last);
    if (pos == string::npos)
      break;
    message << "%" << std::hex << std::uppercase << int(input.at(pos));
    last = pos + 1;
    pos = input.find_first_of(gff3_escape, last);
  } while (true);

  return message.str();
}

inline QString gff3_str_clean(QString input) {
  int pos = input.indexOf('%');
  while (pos != -1) {
    auto ele = char(input.mid(pos + 1, 2).toInt(Q_NULLPTR, 16));
    input.replace(pos, 3, QString(ele));
    pos = input.indexOf('%', pos + 1);
  }
  return input;
}

[[noreturn]] inline void throw_error(const QString message) {
  throw runerror(message.toStdString());
}

inline QString chop(const QString &source, QChar terminator = '.') {
  return source.left(source.indexOf(terminator));
}

inline int extractIntID(const QString &id) {
  return id.mid(id.indexOf(":") + 1).toInt();
}

inline QString extractID(const QString &id) {
  return id.mid(id.indexOf(":") + 1);
}

inline void prepare(QSqlQuery &query, const QString &command) {
  if (!query.prepare(command))
    throw_error("Error preparing query:\n" + query.lastQuery() +
                "\nMessage:\n" + query.lastError().text());
}

inline void execBatch(QSqlQuery &queries) {
  if (!queries.execBatch())
    throw_error("Error executing query:\n" + queries.lastQuery() + "\n" +
                queries.lastError().text());
}

inline void exec(QSqlQuery &query) {
  if (!query.exec())
    throw_error("Error executing query:\n" + query.lastQuery() + "\n" +
                query.lastError().text());
}

inline void exec(const QSqlDatabase &db, const QString &query) {
  if (auto result = db.exec(query); !result.isActive())
    throw_error("Error executing query:\n" + query + "\n" +
                result.lastError().text());
}

inline void exec(const QSqlDatabase &db, const QStringList &queries) {
  for (const auto &query : queries)
    exec(db, query);
}

} // namespace DOGIToys::DOGITools
