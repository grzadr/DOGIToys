#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include <dogitoys/query.hpp>
#include <dogitoys/tools.hpp>

namespace DOGIToys::Mapping {

class UniprotMapRecord {
 private:
  QString gene_id;
  QString transcript_id;
  QString protein_id;
  QString xref;
  QString db_name;
  QString info_type;
  QVariant source_identity;
  QVariant xref_identity;
  QVariant linkage_type;

 public:
  UniprotMapRecord() = delete;
  UniprotMapRecord(const QString &line);

  QString getGeneID() const noexcept { return gene_id; }
  QString getTranscriptID() const noexcept { return transcript_id; }
  QString getProteinID() const noexcept { return protein_id; }
  QString getXRef() const noexcept { return xref; }
  QString getDBName() const noexcept { return db_name; }
  QString getInfoType() const noexcept { return info_type; }

  void insert(QSqlDatabase &db) const;
};

}  // namespace DOGIToys::Mapping
