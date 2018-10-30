#include "test.h"

using General::failed_str;
using General::passed_str;
using General::Stats;
using namespace DOGIToys;
using namespace DOGITools;

Stats EvalDOGIToys::check_gff3_str_escape(bool verbose) {
  Stats stats;

  cout << "~~~ Checking gff3_str_escape function" << endl;

  sstream message;

  int len = 15;

  vector<pair_str> input{{"", ""},
                         //        {";0=","%3B0%3D"},
                         {"=1=3=", "%3D1%3D3%3D"},
                         {"1=3=", "1%3D3%3D"},
                         {"=1=3", "%3D1%3D3"},
                         //        {"0=","0%3D"},
                         //        {";0","%3B0"},
                         {"13", "13"}};

  for (const auto &[query, expected] : input) {
    auto output = gff3_str_escape(query);
    bool result = (output != expected);
    message << std::right << std::setw(3) << ++stats << ")"
            << std::setw(len - 5) << std::quoted(query) << " -> " << std::left
            << std::setw(len) << std::quoted(output)
            << (result ? " != " : " == ") << std::setw(len)
            << std::quoted(expected) << (result ? failed_str : passed_str)
            << "\n";
    stats.add_fail(result);
  }

  if (stats.failed or verbose)
    cout << message.str();

  cout << "~~~ " << gen_summary(stats, "Check") << "\n" << endl;

  return stats;
}

Stats EvalDOGIToys::check_gff3_str_clean(bool verbose) {
  Stats stats;

  cout << "~~~ Checking gff3_str_clean function" << endl;

  sstream message;

  int len = 10;

  vector<pair_str> input{
      {"", ""},
      {"%3B0%3D", ";0="},
      {"0%3D", "0="},
      {"%3B0", ";0"},
      {"0", "0"},
      {"Supporting evidence includes similarity to: 46 ESTs%2C 2 long SRA "
       "reads%2C and 100%25 coverage of the annotated genomic feature by "
       "RNAseq alignments%2C including 9 samples with support for all "
       "annotated introns",
       "Supporting evidence includes similarity to: 46 ESTs, 2 long SRA reads, "
       "and 100% coverage of the annotated genomic feature by RNAseq "
       "alignments, including 9 samples with support for all annotated "
       "introns"}};

  for (const auto &[query, expected] : input) {
    auto output = gff3_str_clean(query);
    bool result = (output != expected);
    message << std::right << std::setw(3) << ++stats << ")" << std::setw(len)
            << std::quoted(query) << " -> " << std::left << std::setw(len)
            << std::quoted(output) << (result ? " != " : " == ")
            << std::setw(len) << std::quoted(expected)
            << (result ? failed_str : passed_str) << "\n";
    stats.add_fail(result);

    const auto qresult =
        gff3_str_clean(QString::fromStdString(query)).toStdString();

    if (expected != qresult) {
      cout << expected << endl;
      cout << qresult << endl;
      cout << result << endl;
      throw runtime_error("QString gff3_clean mismatch");
    }
  }

  if (stats.failed or verbose)
    cout << message.str();

  cout << "~~~ " << gen_summary(stats, "Check") << "\n" << endl;

  return stats;
}

Stats EvalDOGIToys::check_DOGI_basic(QString db_name, bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  cout << "~~~ Checking DOGI: Basic functionality" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Constructing basic DOGI";

  DOGIToys::DOGI db;

  message << passed_str << "\n"
          << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Creating database";

  QFile(db_name).remove();

  db.open(db_name);

  message << passed_str << "\n"
          << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Explicitly closing connection";

  db.close();

  message << passed_str << "\n"
          << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening new connection";

  try {
    db.open(db_name);
    message << passed_str << "\n";
  } catch (const runtime_error &ex) {
    stats.add_fail();
    std::cerr << ex.what() << endl;
    message << failed_str << "\n";
  }

  if (stats.failed or verbose)
    cout << message.str() << endl;

  cout << "~~~ " << gen_summary(stats, "Check") << "\n" << endl;

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_populate(QString db_name,
                                                 QString config_file,
                                                 bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  QFile(db_name).remove();

  cout << "~~~ Checking DOGI: Populating Database" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Constructing DOGI with config file";

  DOGIToys::DOGI db(db_name, config_file);

  if (db.isOpen())
    message << passed_str << "\n";
  else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  db.close();

  if (verbose or stats.failed)
    cout << message.str() << endl;
  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_hpa_expression(const QString &db_name,
                                                       const QString expression,
                                                       bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating Tissue Expression" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating expression";

  try {
    db.populate_hpa_tissue_expression(expression);
    message << passed_str << "\n";
  } catch (const runtime_error &ex) {
    stats.add_fail();
    message << failed_str << "\n";
    cout << ex.what() << endl;
  }

  if (verbose or stats.failed)
    cout << message.str() << endl;

  return stats;
}

General::Stats EvalDOGIToys::check_Dogi_mapper_uniprot(const QString &db_name,
                                                       const QString file_name,
                                                       bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating Mapper" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating mapper";

  try {
    db.populate_mapper("uniprot", "ensembl", file_name);
    message << passed_str << "\n";
  } catch (const runtime_error &ex) {
    stats.add_fail();
    message << failed_str << "\n";
    cout << ex.what() << endl;
  }

  if (verbose or stats.failed)
    cout << message.str() << endl;

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_go_terms(const QString &db_name,
                                                 const QString file_name,
                                                 bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating GOTerms" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating terms";

  try {
    db.populate_go_terms(file_name);
    message << passed_str << "\n";
  } catch (const runtime_error &ex) {
    stats.add_fail();
    message << failed_str << "\n";
    cout << ex.what() << endl;
  }

  if (verbose or stats.failed)
    cout << message.str() << endl;

  //  db.close();

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_go_annotations(const QString &db_name,
                                                       const QString file_name,
                                                       bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating GO Annotations" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating annotations";

  try {
    db.transaction();
    db.populateGOAnnotations(file_name);
    db.commit();
    message << passed_str << "\n";
  } catch (const runtime_error &ex) {
    db.rollback();
    stats.add_fail();
    message << failed_str << "\n";
    cout << ex.what() << endl;
  }

  if (verbose or stats.failed)
    cout << message.str() << endl;

  //  db.close();

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_gene_ontology(
    const QString &db_name, const QString go_terms, const QString &annotations,
    bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating GeneOntology" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating annotations";

  try {
    db.transaction();
    db.populate_go_terms(go_terms, true);
    db.populate_go_annotations(annotations, "ensembl");
    db.commit();
    db.close();
    message << passed_str << "\n";
  } catch (const runtime_error &ex) {
    stats.add_fail();
    message << failed_str << "\n";
    cout << ex.what() << endl;
  }

  if (verbose or stats.failed)
    cout << message.str() << endl;

  //  db.close();

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_mapper(const QString &db_name,
                                               const QString &database_from,
                                               const QString &database_to,
                                               const QString &file_name,
                                               bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating Mapper" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating mapper";

  //  try {
  db.transaction();
  db.populate_mapper(database_from, database_to, file_name);
  db.commit();
  message << passed_str << "\n";
  //  } catch (const runtime_error &ex) {
  //    stats.add_fail();
  //    message << failed_str << "\n";
  //    cout << ex.what() << endl;
  //  }

  if (verbose or stats.failed)
    cout << message.str() << endl;

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_genome(const QString &db_name,
                                               const QString &id_database,
                                               const QString &masking,
                                               const QString &file_name,
                                               bool verbose) {
  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating Genomes" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating Genomes";

  //  try {
  db.transaction();
  db.populate_genomes(id_database, masking, file_name);
  db.commit();
  db.close();
  message << passed_str << "\n";
  //  } catch (const runtime_error &ex) {
  //    stats.add_fail();
  //    message << failed_str << "\n";
  //    cout << ex.what() << endl;
  //  }

  if (verbose or stats.failed)
    cout << message.str() << endl;

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_GVF(const QString &db_name,
                                            const QString &id_database,
                                            const QString &file_name,
                                            bool verbose) {

  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating GVF" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating Genomes";

  //  try {
  db.populateVarStructural(id_database, file_name);
  db.close();
  message << passed_str << "\n";

  if (verbose or stats.failed)
    cout << message.str() << endl;

  return stats;
}

General::Stats EvalDOGIToys::check_DOGI_Regulatory(const QString &db_name,
                                                   const QString &id_database,
                                                   const QString &file_name,
                                                   bool verbose) {

  Stats stats;

  sstream message;

  int width = 67;

  DOGIToys::DOGI db(db_name);

  cout << "~~~ Checking DOGI: Populating Regulatory" << endl;

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Opening connection";

  if (db.isOpen()) {
    message << passed_str << "\n";
  } else {
    stats.add_fail();
    message << failed_str << "\n";
    cout << db.lastError() << endl;
  }

  message << std::right << setw(3) << ++stats << std::left << std::setw(width)
          << ") Populating Regulatory";

  //  try {
  db.initRegulatory();
  db.populateRegulatory(id_database, file_name);
  db.close();
  message << passed_str << "\n";

  if (verbose or stats.failed)
    cout << message.str() << endl;

  return stats;
}

General::Stats EvalDOGIToys::eval_dogitoys(bool verbose) {
  Stats result;

  cout << General::gen_framed("Evaluating Common Tools") << "\n\n";
  result(check_gff3_str_escape(verbose));
  result(check_gff3_str_clean(verbose));
  result(check_DOGI_basic("/data/Git/DOGI/dogi.db", verbose));

  verbose = true;

  //  result(check_DOGI_populate(
  //      "/data/Dropbox/DOGI/Database/cattle.db",
  //      "/data/Dropbox/DOGI/Database/Config/config_cattle.json", verbose));

  //  result(check_DOGI_populate(
  //      "/data/Dropbox/DOGI/Database/dog.db",
  //      "/data/Dropbox/DOGI/Database/Config/config_dog.json", verbose));

  //  result(check_DOGI_populate(
  //      "/data/Dropbox/DOGI/Database/human.db",
  //      "/data/Dropbox/DOGI/Database/Config/config_human.json", verbose));

  //  result(check_DOGI_populate(
  //      "/data/Dropbox/DOGI/Database/pig.db",
  //      "/data/Dropbox/DOGI/Database/Config/config_pig.json", verbose));

  //  result(check_DOGI_populate(
  //      "/data/Dropbox/DOGI/Database/mouse.db",
  //      "/data/Dropbox/DOGI/Database/Config/config_mouse.json", verbose));

  //  result(check_DOGI_populate(
  //      "/data/Dropbox/DOGI/Database/rat.db",
  //      "/data/Dropbox/DOGI/Database/Config/config_rat.json", verbose));

  //  result(check_DOGI_GVF("/data/Dropbox/DOGI/Database/cattle.db", "ensembl",
  //                        "/data/Dropbox/DOGI/Sources/Ensembl/Structural/"
  //                        "bos_taurus_structural_variations.gvf",
  //                        verbose));

  //  result(check_DOGI_Regulatory(
  //      "/data/Dropbox/DOGI/Database/human.db", "ensembl",
  //      "/data/Dropbox/DOGI/Sources/Ensembl/Regulatory/"
  //      "homo_sapiens.GRCh38.Regulatory_Build.regulatory_features.20161111.gff",
  //      verbose));

  //  result(check_DOGI_Regulatory(
  //      "/data/Dropbox/DOGI/Database/mouse.db", "ensembl",
  //      "/data/Dropbox/DOGI/Sources/Ensembl/Regulatory/"
  //      "mus_musculus.GRCm38.Regulatory_Build.regulatory_features.20161111.gff",
  //      verbose));

  cout << General::gen_framed("Evaluation Completed") << "\n\n";

  return result;
}
