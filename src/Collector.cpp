#include "Collector.h"

#include <Rcpp.h>
using namespace Rcpp;

#include "CollectorSkip.h"
#include "CollectorLogical.h"
#include "CollectorInteger.h"
#include "CollectorDouble.h"
#include "CollectorCharacter.h"

CollectorPtr Collector::create(List spec) {
  std::string subclass(as<CharacterVector>(spec.attr("class"))[0]);

  if (subclass == "collector_skip"  )
    return boost::shared_ptr<Collector>(new CollectorSkip());
  if (subclass == "collector_logical"  )
    return boost::shared_ptr<Collector>(new CollectorLogical());
  if (subclass == "collector_integer"  )
    return boost::shared_ptr<Collector>(new CollectorInteger());
  if (subclass == "collector_double"   )
    return boost::shared_ptr<Collector>(new CollectorDouble());
  if (subclass == "collector_character")
    return boost::shared_ptr<Collector>(new CollectorCharacter());

  Rcpp::stop("Unsupported column type");
  return boost::shared_ptr<Collector>();
}

std::vector<CollectorPtr> collectorsCreate(ListOf<List> specs) {
  std::vector<CollectorPtr> collectors;
  for (int j = 0; j < specs.size(); ++j) {
    collectors.push_back(Collector::create(specs[j]));
  }

  return collectors;
}

void collectorsResize(std::vector<CollectorPtr>& collectors, int n) {
  for (int j = 0; j < collectors.size(); ++j) {
    collectors[j]->resize(n);
  }
}

// Guess column types ----------------------------------------------------------

typedef bool (*canParseFun)(std::string);

static bool canParse(CharacterVector x, canParseFun canParse) {
  for (int i = 0; i < x.size(); ++i) {
    if (x[i] == NA_STRING)
      continue;

    if (!canParse(std::string(x[i])))
      return FALSE;
  }
  return TRUE;
}


// [[Rcpp::export]]
std::string collectorGuess(CharacterVector input) {
  // Work from strictest to most flexible
  if (canParse(input, CollectorLogical::canParse))
    return "logical";
  if (canParse(input, CollectorInteger::canParse))
    return "integer";
  if (canParse(input, CollectorDouble::canParse))
    return "double";

  // Otherwise can always parse as a character
  return "character";
}
