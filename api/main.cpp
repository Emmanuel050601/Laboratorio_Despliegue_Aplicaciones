#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <pqxx/pqxx>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

const utility::string_t base_uri = U("http://0.0.0.0:8080");
pqxx::connection conn("dbname=helados user=postgres password=batman2505 host=BD-postgres port=5432");

void handle_get(http_request request) {
    try {

        // Crear una consulta SQL para seleccionar todos los helados
        pqxx::work txn(conn);
        pqxx::result result = txn.exec("SELECT * FROM helado");
        txn.commit();

        // Crear un objeto JSON para almacenar la lista de helados
        json::value response = json::value::object();

        // Crear un arreglo JSON para los helados
        json::value helados = json::value::array();

        // Recorrer los resultados de la consulta y agregar cada helado al arreglo JSON
        int index = 0;
        for (const auto& row : result) {
            json::value helado;
            helado[U("id")] = json::value::number(row[0].as<int>());
            helado[U("nombre")] = json::value::string(row[1].c_str());
            helado[U("stock")] = json::value::number(row[2].as<int>());
            helados[index++] = helado;
        }

        // Agregar el arreglo de helados al objeto JSON de respuesta
        response[U("helados")] = helados;

        // Enviar la respuesta con el objeto JSON que contiene la lista de helados
        http_response http_response(status_codes::OK);
        http_response.headers().set_content_type(U("application/json"));
        http_response.set_body(response);
        request.reply(http_response);
    } catch (const std::exception& e) {
        // Enviar una respuesta de error en caso de una excepción
        http_response http_response(status_codes::InternalError);
        http_response.set_body(U("Error interno en el servidor: ") + utility::conversions::to_string_t(e.what()));
        request.reply(http_response);
    }
}

void handle_post(http_request request) {
    // Extraer los datos JSON de la solicitud
    request.extract_json().then([=](json::value body) {
        try {
            // Verificar que el cuerpo de la solicitud contiene los campos 'nombre' y 'stock'
            if (body.has_field(U("nombre")) && body.has_field(U("stock"))) {
                // Obtener los valores de 'nombre' y 'stock' desde el cuerpo JSON
                utility::string_t nombre = body[U("nombre")].as_string();
                int stock = body[U("stock")].as_integer();

                // Crear y ejecutar una consulta SQL para insertar un nuevo helado
                pqxx::work txn(conn);
                pqxx::result result = txn.exec_params("INSERT INTO helado (name, stock) VALUES ($1, $2) RETURNING id",
                                                      nombre, stock);
                txn.commit();

                // Enviar una respuesta de éxito con el ID del nuevo helado
                json::value response;
                response[U("id")] = json::value::number(result[0][0].as<int>());
                request.reply(status_codes::Created, response);
            } else {
                // Enviar una respuesta de error si falta algún campo en el JSON
                request.reply(status_codes::BadRequest, U("El cuerpo de la solicitud debe contener 'nombre' y 'stock'."));
            }
        } catch (const std::exception& e) {
            // Enviar una respuesta de error en caso de una excepción
            request.reply(status_codes::InternalError, U("Error interno en el servidor: ") + utility::conversions::to_string_t(e.what()));
        }
    });
}


int main() {
    http_listener listener(base_uri);
    listener.support(methods::GET, handle_get);
    listener.support(methods::POST, handle_post);

    try {
        listener.open().wait();
        ucout << U("Listening on ") << base_uri << std::endl;
        while (true);
    } catch (const std::exception& e) {
        ucout << U("Error: ") << e.what() << std::endl;
    }

    return 0;
}

