
#include <xjson/xjson.h>

/*

 {
    "snapshot_date": "Oct 23th, 2017",
    "source"={
        "site": "http://www.imdb.com/title/tt1396484/?ref_=nv_sr_1"
    },
    "movies"=[{
        "title": "It (2017)",
        "actors": [{ "name": "Bill Skarsgård", "role": "Pennywise" }, { "name": "Jaeden Lieberher", "role": "Bill" }],
        "keywords": ["clown", "based on novel", "supernatural", "balloon", "fear"],
        "poster_url": null,
        "reviews": 928
        "rating": 7.8
    }, {
        "title": "Jigsaw (2017)",
        "actors": [{ "name": "Tobin Bell", "role": "John Kramer" }, { "name": "Matt Passmore", "role": "Logan Nelson" }],
        "keywords": ["copycat killer", "one word title", "cop", "murder investigation"],
        "poster_url": "https://images-na.ssl-images-amazon.com/images/M/MV5BNmRiZDM4ZmMtOTVjMi00YTNlLTkyNjMtMjI2OTAxNjgwMWM1XkEyXkFqcGdeQXVyMjMxOTE0ODA@._V1_SY1000_CR0,0,648,1000_AL_.jpg"
    }]
 }

 */

int main(int argc, char* argv[])
{
    xjson_pool_t *pool;
    xjson_json_t *document, *source, *it, *jigsaw, *actors_skarsgard, *actors_lieberher, *actors_bell, *actors_passmore;
    xjson_array_t *movies, *it_actors, *it_keywords, *jigsaw_actors, *jigsaw_keywords;

    xjson_pool_create(&pool);
    xjson_json_create(&document, pool);
    xjson_json_add_property(document, xjson_string, "snapshot_date", "Oct 23th, 2017");
    xjson_json_add_object(&source, document, "source");
        xjson_json_add_property(source, xjson_string, "site", "http://www.imdb.com/title/tt1396484/?ref_=nv_sr_1");
    xjson_json_add_array(&movies, document, xjson_object, "movies");
        xjson_array_add_object(&it, movies);
            xjson_json_add_property(it, xjson_string, "title", "It (2017)");
            xjson_json_add_array(&it_actors, it, xjson_object, "actors");
                xjson_array_add_object(&actors_skarsgard, it_actors);
                    xjson_json_add_property(&actors_skarsgard, xjson_string, "name", "Bill Skarsgård");
                    xjson_json_add_property(&actors_skarsgard, xjson_string, "role", "Pennywise");
                xjson_array_add_object(&actors_lieberher, it_actors);
                    xjson_json_add_property(&actors_lieberher, xjson_string, "name", "Jaeden Lieberher");
                    xjson_json_add_property(&actors_lieberher, xjson_string, "role", "Bill");
            xjson_json_add_array(&it_keywords, it, xjson_string, "keywords");
                xjson_array_add_property(it_keywords, "clown");
                xjson_array_add_property(it_keywords, "based on novel");
                xjson_array_add_property(it_keywords, "supernatural");
                xjson_array_add_property(it_keywords, "balloon");
                xjson_array_add_property(it_keywords, "fear");
            xjson_json_add_property(it, xjson_null, "poster_url", NULL);
            xjson_json_add_property(it, xjson_null, "reviews", 928);
            xjson_json_add_property(it, xjson_null, "rating", &(xjson_double_t){7.8});
        xjson_array_add_object(&jigsaw, movies);
            xjson_json_add_property(jigsaw, xjson_string, "title", "Jigsaw (2017)");
            xjson_json_add_array(&it_actors, jigsaw, xjson_object, "actors");
                xjson_array_add_object(&actors_skarsgard, it_actors);
                    xjson_json_add_property(&actors_skarsgard, xjson_string, "name", "Tobin Bell");
                    xjson_json_add_property(&actors_skarsgard, xjson_string, "role", "John Kramer");
                xjson_array_add_object(&actors_lieberher, it_actors);
                    xjson_json_add_property(&actors_lieberher, xjson_string, "name", "Matt Passmore");
                    xjson_json_add_property(&actors_lieberher, xjson_string, "role", "Logan Nelson");
            xjson_json_add_array(&jigsaw_keywords, jigsaw, xjson_string, "keywords");
                xjson_array_add_property(jigsaw_keywords, "copycat killer");
                xjson_array_add_property(jigsaw_keywords, "one word title");
                xjson_array_add_property(jigsaw_keywords, "cop");
                xjson_array_add_property(jigsaw_keywords, "murder investigation");
            xjson_json_add_property(jigsaw, xjson_string, "poster_url", "https://images-na.ssl-images-amazon.com/images/M/MV5BNmRiZDM4ZmMtOTVjMi00YTNlLTkyNjMtMjI2OTAxNjgwMWM1XkEyXkFqcGdeQXVyMjMxOTE0ODA@._V1_SY1000_CR0,0,648,1000_AL_.jpg");

    xjson_json_print(stdout, document);


}