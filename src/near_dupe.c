#include <stdarg.h>

#include "log/log.h"

#include "near_dupe.h"
#include "double_metaphone.h"
#include "expand.h"
#include "features.h"
#include "float_utils.h"
#include "place.h"
#include "scanner.h"
#include "string_utils.h"
#include "tokens.h"
#include "unicode_scripts.h"
#include "unicode_script_types.h"

#include "geohash/geohash.h"

#define MAX_GEOHASH_PRECISION 12

#define NAME_KEY_PREFIX "n"
#define ADDRESS_KEY_PREFIX "a"
#define UNIT_KEY_PREFIX "u"
#define PO_BOX_KEY_PREFIX "p"
#define HOUSE_NUMBER_KEY_PREFIX "h"
#define STREET_KEY_PREFIX "s"

#define GEOHASH_KEY_PREFIX "gh"
#define POSTCODE_KEY_PREFIX "pc"
#define CITY_KEY_PREFIX "ct"
#define CONTAINING_BOUNDARY_PREFIX "cb"

#define NAME_ADDRESS_UNIT_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_ADDRESS_UNIT_CITY_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_ADDRESS_UNIT_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_ADDRESS_UNIT_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_ADDRESS_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_ADDRESS_CITY_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_ADDRESS_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_ADDRESS_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX ADDRESS_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_HOUSE_NUMBER_UNIT_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_HOUSE_NUMBER_UNIT_CITY_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_HOUSE_NUMBER_UNIT_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_HOUSE_NUMBER_UNIT_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_HOUSE_NUMBER_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_HOUSE_NUMBER_CITY_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_HOUSE_NUMBER_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_HOUSE_NUMBER_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_STREET_UNIT_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_STREET_UNIT_CITY_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_STREET_UNIT_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_STREET_UNIT_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_STREET_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_STREET_CITY_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_STREET_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_STREET_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX STREET_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_PO_BOX_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX PO_BOX_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_PO_BOX_CITY_KEY_PREFIX NAME_KEY_PREFIX PO_BOX_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_PO_BOX_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX PO_BOX_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_PO_BOX_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX PO_BOX_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_UNIT_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX UNIT_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_UNIT_CITY_KEY_PREFIX NAME_KEY_PREFIX UNIT_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_UNIT_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX UNIT_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_UNIT_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX UNIT_KEY_PREFIX POSTCODE_KEY_PREFIX

#define NAME_GEOHASH_KEY_PREFIX NAME_KEY_PREFIX GEOHASH_KEY_PREFIX
#define NAME_CITY_KEY_PREFIX NAME_KEY_PREFIX CITY_KEY_PREFIX
#define NAME_CONTAINING_KEY_PREFIX NAME_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define NAME_POSTCODE_KEY_PREFIX NAME_KEY_PREFIX POSTCODE_KEY_PREFIX

#define ADDRESS_UNIT_GEOHASH_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX GEOHASH_KEY_PREFIX
#define ADDRESS_UNIT_CITY_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX CITY_KEY_PREFIX
#define ADDRESS_UNIT_CONTAINING_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define ADDRESS_UNIT_POSTCODE_KEY_PREFIX ADDRESS_KEY_PREFIX UNIT_KEY_PREFIX POSTCODE_KEY_PREFIX

#define ADDRESS_GEOHASH_KEY_PREFIX ADDRESS_KEY_PREFIX GEOHASH_KEY_PREFIX
#define ADDRESS_CITY_KEY_PREFIX ADDRESS_KEY_PREFIX CITY_KEY_PREFIX
#define ADDRESS_CONTAINING_KEY_PREFIX ADDRESS_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define ADDRESS_POSTCODE_KEY_PREFIX ADDRESS_KEY_PREFIX POSTCODE_KEY_PREFIX

#define HOUSE_NUMBER_UNIT_GEOHASH_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX GEOHASH_KEY_PREFIX
#define HOUSE_NUMBER_UNIT_CITY_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX CITY_KEY_PREFIX
#define HOUSE_NUMBER_UNIT_CONTAINING_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define HOUSE_NUMBER_UNIT_POSTCODE_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX UNIT_KEY_PREFIX POSTCODE_KEY_PREFIX

#define HOUSE_NUMBER_GEOHASH_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX GEOHASH_KEY_PREFIX
#define HOUSE_NUMBER_CITY_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX CITY_KEY_PREFIX
#define HOUSE_NUMBER_CONTAINING_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define HOUSE_NUMBER_POSTCODE_KEY_PREFIX HOUSE_NUMBER_KEY_PREFIX POSTCODE_KEY_PREFIX

#define STREET_GEOHASH_KEY_PREFIX STREET_KEY_PREFIX GEOHASH_KEY_PREFIX
#define STREET_CITY_KEY_PREFIX STREET_KEY_PREFIX CITY_KEY_PREFIX
#define STREET_CONTAINING_KEY_PREFIX STREET_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define STREET_POSTCODE_KEY_PREFIX STREET_KEY_PREFIX POSTCODE_KEY_PREFIX

#define STREET_UNIT_GEOHASH_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX GEOHASH_KEY_PREFIX
#define STREET_UNIT_CITY_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX CITY_KEY_PREFIX
#define STREET_UNIT_CONTAINING_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define STREET_UNIT_POSTCODE_KEY_PREFIX STREET_KEY_PREFIX UNIT_KEY_PREFIX POSTCODE_KEY_PREFIX

#define PO_BOX_GEOHASH_KEY_PREFIX PO_BOX_KEY_PREFIX GEOHASH_KEY_PREFIX
#define PO_BOX_CITY_KEY_PREFIX PO_BOX_KEY_PREFIX CITY_KEY_PREFIX
#define PO_BOX_CONTAINING_KEY_PREFIX PO_BOX_KEY_PREFIX CONTAINING_BOUNDARY_PREFIX
#define PO_BOX_POSTCODE_KEY_PREFIX PO_BOX_KEY_PREFIX POSTCODE_KEY_PREFIX

cstring_array *expanded_component_combined(char *input, libpostal_normalize_options_t options, size_t *n) {
    size_t num_expansions = 0;
    cstring_array *expansions = expand_address(input, options, &num_expansions);

    size_t num_root_expansions = 0;
    cstring_array *root_expansions = expand_address_root(input, options, &num_root_expansions);
    
    if (num_root_expansions == 0) {
        cstring_array_destroy(root_expansions);
        *n = num_expansions;
        return expansions;
    } else if (num_expansions == 0) {
        cstring_array_destroy(expansions);
        *n = num_root_expansions;
        return root_expansions;
    } else {
        khash_t(str_set) *unique_strings = kh_init(str_set);
        char *expansion;
        khiter_t k;
        int ret;

        cstring_array *all_expansions = cstring_array_new();

        for (size_t i = 0; i < num_expansions; i++) {
            expansion = cstring_array_get_string(expansions, i);
            k = kh_get(str_set, unique_strings, expansion);

            if (k == kh_end(unique_strings)) {
                cstring_array_add_string(all_expansions, expansion);
                k = kh_put(str_set, unique_strings, expansion, &ret);
                if (ret < 0) {
                    break;
                }
            }
        }

        for (size_t i = 0; i < num_root_expansions; i++) {
            expansion = cstring_array_get_string(root_expansions, i);
            k = kh_get(str_set, unique_strings, expansion);

            if (k == kh_end(unique_strings)) {
                cstring_array_add_string(all_expansions, expansion);
                k = kh_put(str_set, unique_strings, expansion, &ret);
                if (ret < 0) {
                    break;
                }
            }
        }

        *n = cstring_array_num_strings(all_expansions);

        kh_destroy(str_set, unique_strings);
        cstring_array_destroy(root_expansions);
        cstring_array_destroy(expansions);

        return all_expansions;
    }       
}

static inline cstring_array *expanded_component_root_with_fallback(char *input, libpostal_normalize_options_t options, size_t *n) {
    cstring_array *root_expansions = expand_address_root(input, options, n);
    if (*n > 0) {
        return root_expansions;
    } else {
        cstring_array_destroy(root_expansions);
        *n = 0;
        return expand_address(input, options, n);
    }
}


static cstring_array *geohash_and_neighbors(double latitude, double longitude, size_t geohash_precision) {
    if (geohash_precision == 0) return NULL;

    if (geohash_precision > MAX_GEOHASH_PRECISION) geohash_precision = MAX_GEOHASH_PRECISION;

    char geohash[geohash_precision + 1];
    if (geohash_encode(latitude, longitude, geohash, geohash_precision) != GEOHASH_OK) {
        return NULL;
    }

    size_t neighbors_size = geohash_precision * 8 + 1;
    char neighbors[neighbors_size];

    int num_strings = 0;

    if (geohash_neighbors(geohash, neighbors, neighbors_size, &num_strings) == GEOHASH_OK && num_strings == 8) {
        cstring_array *strings = cstring_array_new_size(9 * geohash_precision + 1);
        cstring_array_add_string(strings, geohash);

        for (int i = 0; i < num_strings; i++) {
            char *neighbor = neighbors + geohash_precision * i;
            cstring_array_add_string(strings, neighbor);
        }
        return strings;
    }

    return NULL;
}


cstring_array *name_word_hashes(char *name, libpostal_normalize_options_t normalize_options) {
    normalize_options.address_components = LIBPOSTAL_ADDRESS_NAME | LIBPOSTAL_ADDRESS_ANY;
    size_t num_expansions = 0;
    cstring_array *name_expansions = expanded_component_root_with_fallback(name, normalize_options, &num_expansions);
    if (num_expansions == 0) {
        cstring_array_destroy(name_expansions);
        return NULL;
    }

    size_t len = strlen(name);

    char_array *token_string_array = char_array_new_size(len);
    cstring_array *strings = cstring_array_new_size(len);
    token_array *token_array = token_array_new();

    char_array *combined_words_no_whitespace = char_array_new();

    bool keep_whitespace = false;

    khash_t(str_set) *unique_strings = kh_init(str_set);
    khiter_t k;
    int ret = 0;

    for (size_t i = 0; i < num_expansions; i++) {
        char *expansion = cstring_array_get_string(name_expansions, i);
        log_debug("expansion = %s\n", expansion);
        tokenize_add_tokens(token_array, expansion, strlen(expansion), keep_whitespace);
        size_t num_tokens = token_array->n;
        token_t *tokens = token_array->a;
        token_t prev_token;
        char *token_str;
        for (size_t j = 0; j < num_tokens; j++) {
            token_t token = tokens[j];
            bool ideogram = is_ideographic(token.type);

            string_script_t token_script = get_string_script(expansion + token.offset, token.len);
            bool is_latin = token_script.len == token.len && token_script.script == SCRIPT_LATIN;

            char_array_clear(token_string_array);
            // For ideograms, since the "words" are characters, we use shingles of two characters
            if (ideogram && j > 0 && is_ideographic(prev_token.type)) {
                log_debug("cat ideogram\n");
                char_array_cat_len(token_string_array, expansion + prev_token.offset, prev_token.len);
            }

            // For Latin script, add double metaphone of the words
            if (is_latin && !is_numeric_token(token.type) && !ideogram && !is_punctuation(token.type)) {
                char_array_clear(token_string_array);
                char_array_cat_len(token_string_array, expansion + token.offset, token.len);
                token_str = char_array_get_string(token_string_array);

                log_debug("token_str = %s\n", token_str);

                double_metaphone_codes_t *dm_codes = double_metaphone(token_str);
                if (dm_codes == NULL) {
                    prev_token = token;
                    continue;
                }
                char *dm_primary = dm_codes->primary;
                char *dm_secondary = dm_codes->secondary;

                if (!string_equals(dm_primary, "")) {

                    k = kh_get(str_set, unique_strings, dm_primary);

                    if (k == kh_end(unique_strings)) {
                        log_debug("adding dm_primary = %s\n", dm_primary);
                        cstring_array_add_string(strings, dm_primary);
                        k = kh_put(str_set, unique_strings, strdup(dm_primary), &ret);
                        if (ret < 0) {
                            break;
                        }
                    }

                    if (!string_equals(dm_secondary, dm_primary)) {

                        k = kh_get(str_set, unique_strings, dm_secondary);

                        if (k == kh_end(unique_strings)) {
                            log_debug("adding dm_secondary = %s\n", dm_secondary);
                            cstring_array_add_string(strings, dm_secondary);
                            k = kh_put(str_set, unique_strings, strdup(dm_secondary), &ret);
                            if (ret < 0) {
                                break;
                            }
                        }
                    }
                }
                double_metaphone_codes_destroy(dm_codes);
            // For non-Latin words (Arabic, Cyrllic, etc.) just add the word
            // For ideograms, we do two-character shingles, so only add the first character if the string has one token
            } else if (!ideogram || j > 0 || num_tokens == 1) {
                char_array_cat_len(token_string_array, expansion + token.offset, token.len);
                token_str = char_array_get_string(token_string_array);
                log_debug("token_str = %s\n", token_str);
                k = kh_get(str_set, unique_strings, token_str);

                if (k == kh_end(unique_strings)) {
                    cstring_array_add_string(strings, token_str);
                    k = kh_put(str_set, unique_strings, strdup(token_str), &ret);
                    if (ret < 0) {
                        break;
                    }
                }
            }

            prev_token = token;
        }

        token_array_clear(token_array);
    }

    char_array_destroy(token_string_array);
    token_array_destroy(token_array);
    char_array_destroy(combined_words_no_whitespace);

    const char *key;

    kh_foreach_key(unique_strings, key, {
        free((char *)key);
    });
    kh_destroy(str_set, unique_strings);

    return strings;
}


static inline void add_string_arrays_to_tree(string_tree_t *tree, size_t n, va_list args) {
    for (size_t i = 0; i < n; i++) {
        cstring_array *a = va_arg(args, cstring_array *);
        size_t num_strings = cstring_array_num_strings(a);
        if (num_strings == 0) continue;
        for (size_t j = 0; j < num_strings; j++) {
            char *str = cstring_array_get_string(a, j);
            string_tree_add_string(tree, str);
        }
        string_tree_finalize_token(tree);
    }
    va_end(args);
}

static inline void add_hashes_from_tree(cstring_array *near_dupe_hashes, char *prefix, string_tree_t *tree) {
    string_tree_iterator_t *iter = string_tree_iterator_new(tree);
    if (iter->num_tokens > 0) {
        log_debug("iter->num_tokens = %zu\n", iter->num_tokens);

        for (; !string_tree_iterator_done(iter); string_tree_iterator_next(iter)) {

            cstring_array_start_token(near_dupe_hashes);
            cstring_array_append_string(near_dupe_hashes, prefix);

            char *str;
            string_tree_iterator_foreach_token(iter, str, {
                cstring_array_append_string(near_dupe_hashes, "|");
                cstring_array_append_string(near_dupe_hashes, str);
                //log_debug("str=%s\n", str);
            });

            cstring_array_terminate(near_dupe_hashes);
        }
    }

    string_tree_iterator_destroy(iter);
}


static inline void add_string_hash_permutations(cstring_array *near_dupe_hashes, char *prefix, string_tree_t *tree, size_t n, ...) {
    string_tree_clear(tree);

    log_debug("prefix=%s\n", prefix);

    va_list args;
    va_start(args, n);
    add_string_arrays_to_tree(tree, n, args);
    va_end(args);

    log_debug("string_tree_num_strings(tree)=%zu\n", string_tree_num_strings(tree));

    add_hashes_from_tree(near_dupe_hashes, prefix, tree);
}


cstring_array *near_dupe_hashes_languages(size_t num_components, char **labels, char **values, libpostal_near_dupe_hash_options_t options, size_t num_languages, char **languages) {
    place_t *place = place_from_components(num_components, labels, values);
    log_debug("created place\n");
    if (place == NULL) return NULL;

    size_t n = 0;

    libpostal_normalize_options_t normalize_options = libpostal_get_default_options();

    language_classifier_response_t *lang_response = NULL;

    if (num_languages == 0) {
        lang_response = place_languages(num_components, labels, values);

         if (lang_response != NULL) {
            log_debug("got %zu place languages\n", lang_response->num_languages);
            normalize_options.num_languages = lang_response->num_languages;
            normalize_options.languages = lang_response->languages;
         }
    } else {
        normalize_options.num_languages = languages;
        normalize_options.languages = languages;
    }

    string_tree_t *tree = string_tree_new();

    cstring_array *name_expansions = NULL;
    size_t num_name_expansions = 0;
    if (place->name != NULL && options.with_name) {
        log_debug("Doing name expansions for %s\n", place->name);
        name_expansions = name_word_hashes(place->name, normalize_options);
        if (name_expansions != NULL) {
            num_name_expansions = cstring_array_num_strings(name_expansions);
            log_debug("Got %zu name expansions\n", num_name_expansions);
        }
    }


    cstring_array *street_expansions = NULL;
    size_t num_street_expansions = 0;
    if (place->street != NULL) {
        log_debug("Doing street expansions for %s\n", place->street);
        normalize_options.address_components = LIBPOSTAL_ADDRESS_STREET | LIBPOSTAL_ADDRESS_ANY;
        street_expansions = expanded_component_combined(place->street, normalize_options, &num_street_expansions);
        log_debug("Got %zu street expansions\n", num_street_expansions);
    }

    cstring_array *house_number_expansions = NULL;
    size_t num_house_number_expansions = 0;
    if (place->house_number != NULL) {
        log_debug("Doing house number expansions for %s\n", place->house_number);
        normalize_options.address_components = LIBPOSTAL_ADDRESS_HOUSE_NUMBER | LIBPOSTAL_ADDRESS_ANY;
        house_number_expansions = expand_address_root(place->house_number, normalize_options, &num_house_number_expansions);
        log_debug("Got %zu house number expansions\n", num_house_number_expansions);
    }

    cstring_array *unit_expansions = NULL;
    size_t num_unit_expansions = 0;
    if (place->unit != NULL && options.with_unit) {
        log_debug("Doing unit expansions for %s\n", place->unit);
        normalize_options.address_components = LIBPOSTAL_ADDRESS_UNIT | LIBPOSTAL_ADDRESS_ANY;
        unit_expansions = expand_address_root(place->unit, normalize_options, &num_unit_expansions);
        log_debug("Got %zu unit expansions\n", num_unit_expansions);
    }

    cstring_array *building_expansions = NULL;
    size_t num_building_expansions = 0;
    if (place->building != NULL && options.with_unit) {
        normalize_options.address_components = LIBPOSTAL_ADDRESS_UNIT | LIBPOSTAL_ADDRESS_ANY;
        building_expansions = expand_address_root(place->building, normalize_options, &num_building_expansions);
    }

    cstring_array *level_expansions = NULL;
    size_t num_level_expansions = 0;
    if (place->level != NULL && options.with_unit) {
        normalize_options.address_components = LIBPOSTAL_ADDRESS_LEVEL | LIBPOSTAL_ADDRESS_ANY;
        level_expansions = expand_address_root(place->level, normalize_options, &num_level_expansions);
    }

    cstring_array *po_box_expansions = NULL;
    size_t num_po_box_expansions = 0;
    if (place->po_box != NULL) {
        normalize_options.address_components = LIBPOSTAL_ADDRESS_PO_BOX | LIBPOSTAL_ADDRESS_ANY;
        po_box_expansions = expand_address_root(place->po_box, normalize_options, &num_po_box_expansions);
    }

    cstring_array *place_expansions = NULL;
    cstring_array *containing_expansions = NULL;

    if (options.with_city_or_equivalent) {
        normalize_options.address_components = LIBPOSTAL_ADDRESS_TOPONYM | LIBPOSTAL_ADDRESS_ANY;

        if (place->city != NULL) {
            size_t num_city_expansions = 0;
            cstring_array *city_expansions = expand_address_root(place->city, normalize_options, &num_city_expansions);
            if (place_expansions == NULL) {
                place_expansions = city_expansions;
            } else if (city_expansions != NULL && num_city_expansions > 0) {
                cstring_array_extend(place_expansions, city_expansions);
                cstring_array_destroy(city_expansions);
            }

        }

        if (place->city_district != NULL) {
            size_t num_city_district_expansions = 0;
            cstring_array *city_district_expansions = expand_address_root(place->city_district, normalize_options, &num_city_district_expansions);
            if (place_expansions == NULL) {
                place_expansions = city_district_expansions;
            } else if (city_district_expansions != NULL && num_city_district_expansions > 0) {
                cstring_array_extend(place_expansions, city_district_expansions);
                cstring_array_destroy(city_district_expansions);
            }
        }

        if (place->suburb != NULL) {
            size_t num_suburb_expansions = 0;
            cstring_array *suburb_expansions = expand_address_root(place->suburb, normalize_options, &num_suburb_expansions);
            if (place_expansions == NULL) {
                place_expansions = suburb_expansions;
            } else if (suburb_expansions != NULL && num_suburb_expansions > 0) {
                cstring_array_extend(place_expansions, suburb_expansions);
                cstring_array_destroy(suburb_expansions);
            }
        }


        if (place->island != NULL) {
            size_t num_island_expansions = 0;
            cstring_array *island_expansions = expand_address_root(place->island, normalize_options, &num_island_expansions);
            if (place_expansions == NULL) {
                place_expansions = island_expansions;
            } else if (island_expansions != NULL && num_island_expansions > 0) {
                cstring_array_extend(place_expansions, island_expansions);
                cstring_array_destroy(island_expansions);
            }
        }

        if (place->state_district != NULL) {
            size_t num_state_district_expansions = 0;
            cstring_array *state_district_expansions = expand_address_root(place->state_district, normalize_options, &num_state_district_expansions);
            if (containing_expansions == NULL) {
                containing_expansions = state_district_expansions;
            } else if (state_district_expansions != NULL && num_state_district_expansions > 0) {
                cstring_array_extend(containing_expansions, state_district_expansions);
                cstring_array_destroy(state_district_expansions);
            }
        }
    }

    cstring_array *postal_code_expansions = NULL;
    size_t num_postal_code_expansions = 0;
    if (options.with_postal_code && place->postal_code != NULL) {
        normalize_options.address_components = LIBPOSTAL_ADDRESS_POSTAL_CODE | LIBPOSTAL_ADDRESS_ANY;
        postal_code_expansions = expand_address_root(place->postal_code, normalize_options, &num_postal_code_expansions);
    }

    cstring_array *geohash_expansions = NULL;
    if (options.with_latlon && !(double_equals(options.latitude, 0.0) && double_equals(options.longitude, 0.0))) {
        geohash_expansions = geohash_and_neighbors(options.latitude, options.longitude, options.geohash_precision);
    }

    size_t num_geohash_expansions = geohash_expansions != NULL ? cstring_array_num_strings(geohash_expansions) : 0;
    if (num_geohash_expansions == 0 && num_postal_code_expansions == 0 && place_expansions == NULL && containing_expansions == NULL) {
        return NULL;
    }

    bool added = false;

    num_name_expansions = name_expansions != NULL ? cstring_array_num_strings(name_expansions) : 0;
    num_street_expansions = street_expansions != NULL ? cstring_array_num_strings(street_expansions) : 0;
    num_house_number_expansions = house_number_expansions != NULL ? cstring_array_num_strings(house_number_expansions) : 0;
    num_po_box_expansions = po_box_expansions != NULL ? cstring_array_num_strings(po_box_expansions) : 0;
    num_unit_expansions = unit_expansions != NULL ? cstring_array_num_strings(unit_expansions) : 0;
    num_building_expansions = building_expansions != NULL ? cstring_array_num_strings(building_expansions) : 0;
    num_level_expansions = level_expansions != NULL ? cstring_array_num_strings(level_expansions) : 0;

    bool have_unit = num_unit_expansions > 0 || num_building_expansions > 0 || num_level_expansions > 0;
    cstring_array *unit_or_equivalent_expansions = NULL;
    if (num_unit_expansions > 0) {
        unit_or_equivalent_expansions = unit_expansions;
    } else if (num_building_expansions > 0) {
        unit_or_equivalent_expansions = building_expansions;
    } else if (num_level_expansions > 0) {
        unit_or_equivalent_expansions = level_expansions;
    }

    cstring_array *near_dupe_hashes = cstring_array_new();

    if (num_name_expansions > 0) {
        if (num_street_expansions > 0 && num_house_number_expansions > 0 && options.name_and_address_keys) {
            // Have street, house number, and unit
            if (have_unit) {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_UNIT_GEOHASH_KEY_PREFIX, tree, 5, name_expansions, street_expansions, house_number_expansions, unit_or_equivalent_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_UNIT_CITY_KEY_PREFIX, tree, 5, name_expansions, street_expansions, house_number_expansions, unit_or_equivalent_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_UNIT_CONTAINING_KEY_PREFIX, tree, 5, name_expansions, street_expansions, house_number_expansions, unit_or_equivalent_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_UNIT_POSTCODE_KEY_PREFIX, tree, 5, name_expansions, street_expansions, house_number_expansions, unit_or_equivalent_expansions, postal_code_expansions);
                }
            // Have street and house number, no unit
            } else {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_GEOHASH_KEY_PREFIX, tree, 4, name_expansions, street_expansions, house_number_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_CITY_KEY_PREFIX, tree, 4, name_expansions, street_expansions, house_number_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_CONTAINING_KEY_PREFIX, tree, 4, name_expansions, street_expansions, house_number_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_ADDRESS_POSTCODE_KEY_PREFIX, tree, 4, name_expansions, street_expansions, house_number_expansions, postal_code_expansions);
                }
            }
        // Japan, other places with no street names
        } else if (num_house_number_expansions > 0 && options.name_and_address_keys) {
            // House number and unit
            if (have_unit) {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_UNIT_GEOHASH_KEY_PREFIX, tree, 4, name_expansions, house_number_expansions, unit_or_equivalent_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_UNIT_CITY_KEY_PREFIX, tree, 4, name_expansions, house_number_expansions, unit_or_equivalent_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_UNIT_CONTAINING_KEY_PREFIX, tree, 4, name_expansions, house_number_expansions, unit_or_equivalent_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_UNIT_POSTCODE_KEY_PREFIX, tree, 4, name_expansions, house_number_expansions, unit_or_equivalent_expansions, postal_code_expansions);
                }
            // House number, no unit
            } else {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_GEOHASH_KEY_PREFIX, tree, 3, name_expansions, house_number_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_CITY_KEY_PREFIX, tree, 3, name_expansions, house_number_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_CONTAINING_KEY_PREFIX, tree, 3, name_expansions, house_number_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_HOUSE_NUMBER_POSTCODE_KEY_PREFIX, tree, 3, name_expansions, house_number_expansions, postal_code_expansions);
                }
            }
        // Addresses in India, UK, Ireland, many university addresses, etc. may have house name + street with no house numbers
        } else if (num_street_expansions > 0 && options.name_and_address_keys) {
            // Have street, house number, and unit
            if (have_unit) {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_UNIT_GEOHASH_KEY_PREFIX, tree, 4, name_expansions, street_expansions, unit_or_equivalent_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_UNIT_CITY_KEY_PREFIX, tree, 4, name_expansions, street_expansions, unit_or_equivalent_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_UNIT_CONTAINING_KEY_PREFIX, tree, 4, name_expansions, street_expansions, unit_or_equivalent_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_UNIT_POSTCODE_KEY_PREFIX, tree, 4, name_expansions, street_expansions, unit_or_equivalent_expansions, postal_code_expansions);
                }
            // Have street and house number, no unit
            } else {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_GEOHASH_KEY_PREFIX, tree, 3, name_expansions, street_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_CITY_KEY_PREFIX, tree, 3, name_expansions, street_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_CONTAINING_KEY_PREFIX, tree, 3, name_expansions, street_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_STREET_POSTCODE_KEY_PREFIX, tree, 3, name_expansions, street_expansions, postal_code_expansions);
                }
            }
        // PO Box only addresses, mailing addresses
        } else if (num_po_box_expansions > 0 && options.name_and_address_keys) {
            if (geohash_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, NAME_PO_BOX_GEOHASH_KEY_PREFIX, tree, 3, name_expansions, po_box_expansions, geohash_expansions);
            }
            if (place_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, NAME_PO_BOX_CITY_KEY_PREFIX, tree, 3, name_expansions, po_box_expansions, place_expansions);
            }

            if (containing_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, NAME_PO_BOX_CONTAINING_KEY_PREFIX, tree, 3, name_expansions, po_box_expansions, containing_expansions);
            }

            if (postal_code_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, NAME_PO_BOX_POSTCODE_KEY_PREFIX, tree, 3, name_expansions, po_box_expansions, postal_code_expansions);
            }
        // Only name
        } else if (options.name_only_keys) {
            // Have name and unit, some university addresses
            if (have_unit) {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_UNIT_GEOHASH_KEY_PREFIX, tree, 3, name_expansions, unit_or_equivalent_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_UNIT_CITY_KEY_PREFIX, tree, 3, name_expansions, unit_or_equivalent_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_UNIT_CONTAINING_KEY_PREFIX, tree, 3, name_expansions, unit_or_equivalent_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_UNIT_POSTCODE_KEY_PREFIX, tree, 3, name_expansions, unit_or_equivalent_expansions, postal_code_expansions);
                }
            // Have name and geo only
            } else {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_GEOHASH_KEY_PREFIX, tree, 2, name_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_CITY_KEY_PREFIX, tree, 2, name_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_CONTAINING_KEY_PREFIX, tree, 2, name_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, NAME_POSTCODE_KEY_PREFIX, tree, 2, name_expansions, postal_code_expansions);
                }
            }
        }
    }

    if (options.address_only_keys) {
        if (num_street_expansions > 0 && num_house_number_expansions > 0) {
            // Have street, house number, and unit
            if (have_unit) {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_UNIT_GEOHASH_KEY_PREFIX, tree, 4, street_expansions, house_number_expansions, unit_or_equivalent_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_UNIT_CITY_KEY_PREFIX, tree, 4, street_expansions, house_number_expansions, unit_or_equivalent_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_UNIT_CONTAINING_KEY_PREFIX, tree, 4, street_expansions, house_number_expansions, unit_or_equivalent_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_UNIT_POSTCODE_KEY_PREFIX, tree, 4, street_expansions, house_number_expansions, unit_or_equivalent_expansions, postal_code_expansions);
                }
            // Have street and house number, no unit
            } else {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_GEOHASH_KEY_PREFIX, tree, 3, street_expansions, house_number_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_CITY_KEY_PREFIX, tree, 3, street_expansions, house_number_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_CONTAINING_KEY_PREFIX, tree, 3, street_expansions, house_number_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, ADDRESS_POSTCODE_KEY_PREFIX, tree, 3, street_expansions, house_number_expansions, postal_code_expansions);
                }
            }
        // Japan, other places with no street names
        } else if (num_house_number_expansions > 0) {
            // House number and unit
            if (have_unit) {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_UNIT_GEOHASH_KEY_PREFIX, tree, 3, house_number_expansions, unit_or_equivalent_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_UNIT_CITY_KEY_PREFIX, tree, 3, house_number_expansions, unit_or_equivalent_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_UNIT_CONTAINING_KEY_PREFIX, tree, 3, house_number_expansions, unit_or_equivalent_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_UNIT_POSTCODE_KEY_PREFIX, tree, 3, house_number_expansions, unit_or_equivalent_expansions, postal_code_expansions);
                }
            // House number, no unit
            } else {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_GEOHASH_KEY_PREFIX, tree, 2, house_number_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_CITY_KEY_PREFIX, tree, 2, house_number_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_CONTAINING_KEY_PREFIX, tree, 2, house_number_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, HOUSE_NUMBER_POSTCODE_KEY_PREFIX, tree, 2, house_number_expansions, postal_code_expansions);
                }
            }
        // Addresses in India, UK, Ireland, many university addresses, etc. may have house name + street with no house numbers
        } else if (num_street_expansions > 0) {
            // Have street, house number, and unit
            if (have_unit) {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_UNIT_GEOHASH_KEY_PREFIX, tree, 3, street_expansions, unit_or_equivalent_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_UNIT_CITY_KEY_PREFIX, tree, 3, street_expansions, unit_or_equivalent_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_UNIT_CONTAINING_KEY_PREFIX, tree, 3, street_expansions, unit_or_equivalent_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_UNIT_POSTCODE_KEY_PREFIX, tree, 3, street_expansions, unit_or_equivalent_expansions, postal_code_expansions);
                }
            // Have street and house number, no unit
            } else {
                if (geohash_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_GEOHASH_KEY_PREFIX, tree, 2, street_expansions, geohash_expansions);
                }

                if (place_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_CITY_KEY_PREFIX, tree, 2, street_expansions, place_expansions);
                }

                if (containing_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_CONTAINING_KEY_PREFIX, tree, 2, street_expansions, containing_expansions);
                }

                if (postal_code_expansions != NULL) {
                    add_string_hash_permutations(near_dupe_hashes, STREET_POSTCODE_KEY_PREFIX, tree, 2, street_expansions, postal_code_expansions);
                }
            }
        // PO Box only addresses, mailing addresses
        } else if (num_po_box_expansions > 0) {
            if (geohash_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, PO_BOX_GEOHASH_KEY_PREFIX, tree, 2, po_box_expansions, geohash_expansions);
            }

            if (place_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, PO_BOX_CITY_KEY_PREFIX, tree, 2, po_box_expansions, place_expansions);
            }

            if (containing_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, PO_BOX_CONTAINING_KEY_PREFIX, tree, 2, po_box_expansions, containing_expansions);
            }

            if (postal_code_expansions != NULL) {
                add_string_hash_permutations(near_dupe_hashes, PO_BOX_POSTCODE_KEY_PREFIX, tree, 2, po_box_expansions, postal_code_expansions);
            }
        }

    }

    if (tree != NULL) {
        string_tree_destroy(tree);
    }

    if (name_expansions != NULL) {
        cstring_array_destroy(name_expansions);
    }

    if (street_expansions != NULL) {
        cstring_array_destroy(street_expansions);
    }

    if (house_number_expansions != NULL) {
        cstring_array_destroy(house_number_expansions);
    }

    if (unit_expansions != NULL) {
        cstring_array_destroy(unit_expansions);
    }

    if (building_expansions != NULL) {
        cstring_array_destroy(building_expansions);
    }

    if (level_expansions != NULL) {
        cstring_array_destroy(level_expansions);
    }

    if (po_box_expansions != NULL) {
        cstring_array_destroy(po_box_expansions);
    }

    if (place_expansions != NULL) {
        cstring_array_destroy(place_expansions);
    }


    if (containing_expansions != NULL) {
        cstring_array_destroy(containing_expansions);
    }

    if (postal_code_expansions != NULL) {
        cstring_array_destroy(postal_code_expansions);
    }

    if (geohash_expansions != NULL) {
        cstring_array_destroy(geohash_expansions);
    }

    if (lang_response != NULL) {
        language_classifier_response_destroy(lang_response);
    }

    return near_dupe_hashes;
}

inline cstring_array *near_dupe_hashes(size_t num_components, char **labels, char **values, libpostal_near_dupe_hash_options_t options) {
    return near_dupe_hashes_languages(num_components, labels, values, options, 0, NULL);
}
