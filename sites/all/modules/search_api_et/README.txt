


INSTALLATION


Simply save this module's subdir in your contrib module directory,
and enable the module.



USAGE


Simply configure your search api index(es) to index the field
"Multilingual full text". Now, all of your selected entity's translations will
be included in your index(es) and the search.



MODULE DESCRIPTION


Search API Entity Translation is a minimalist approach of making multilingual
content managed via entity translations searchable via Search API.
The current approach simply offers a new search API field named
"Multilingual full text" which indexes all of your entity translations.
This is, of course, not very intelligent (since the translations/languages of
content cannot be properly distinguished in the search, let alone searched
language _specificly_ or independently) - but it's a first simply approach.

Search API Entity Translation is written for Drupal 7 and used in production
environments. It is currently only thouroughly tested and used with node based
indexes, though.

Requirements:

* Search API is obviously required, as well as
  a compatible search module (e.g. apachesolr, search_api).

Similar projects:

* Currently (2012-01-02) there does not seem to be other similar modules.



TODOS


* Make different language versions of a content distinguishable for search.



MODULE URL


More information and issues, see the module page, currently at:

  http://drupal.org/project/search_api_et
