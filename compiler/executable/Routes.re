open Httpaf;
open Kore;
open Kore.Compiler;

module Response = Server_Response;
module Generator = KnotGenerate.Generator;

let read_to_string = (req_d, f) => {
  let req_body = Reqd.request_body(req_d);
  let str = ref("");

  let rec read = () =>
    Body.schedule_read(
      req_body,
      ~on_eof=() => f(str^),
      ~on_read=
        (data, ~off, ~len) => {
          let str_part = Bigstring.to_string(~off, ~len, data);
          str := str^ ++ str_part;

          /* continue to read until body is empty */
          read();
        },
    );

  read();
};

let put_module = (compiler, req_d, uri) =>
  read_to_string(
    req_d,
    module_path => {
      Log.info(
        "%s  [PUT %s] (%s)",
        Emoji.right_arrow,
        Uri.to_string(uri),
        module_path,
      );

      Reqd.respond_with_string(
        req_d,
        Response.ok(),
        "module added to context",
      );

      compiler.add_rec(module_path);
    },
  );

let get_module = (compiler, req_d, uri) =>
  read_to_string(
    req_d,
    module_path => {
      Log.info(
        "%s  [GET %s] (%s)",
        Emoji.left_arrow,
        Uri.to_string(uri),
        module_path,
      );

      compiler.find(module_path)
      |> (
        fun
        | Some(ast) => {
            /* let res_body =
               Response.ok() |> Reqd.respond_with_streaming(req_d); */

            let body = ref("");
            Generator.generate(str => body := body^ ++ str, ast);

            /* Body.close_writer(res_body); */

            Log.info("%s  (%s)", Emoji.right_arrow, body^);

            Reqd.respond_with_string(req_d, Response.ok(), body^);
          }
        | None =>
          Reqd.respond_with_string(
            req_d,
            Response.not_found(),
            "no compiled module found",
          )
      );
    },
  );

let invalidate_module = (compiler, req_d, uri) =>
  read_to_string(
    req_d,
    module_path => {
      Log.info(
        "%s  [DELETE %s] (%s)",
        Emoji.cross_mark,
        Uri.to_string(uri),
        module_path,
      );

      compiler.invalidate(module_path);

      Reqd.respond_with_string(req_d, Response.ok(), "module invalidated");
    },
  );

let invalidate_context = (compiler, req_d, uri) => {
  Log.info(
    "%s  [DELETE %s]",
    Emoji.skull_and_crossbones,
    Uri.to_string(uri),
  );

  compiler.reset();

  Reqd.respond_with_string(req_d, Response.ok(), "build context invalidated");
};

let get_status = (compiler, req_d, uri) => {
  Log.info("%s  [GET %s]", Emoji.left_arrow, Uri.to_string(uri));

  let status =
    switch (compiler.status()) {
    | Idle => "idle"
    | Running => "running"
    | Complete => "complete"
    };

  Log.info("%s  (%s)", Emoji.vertical_traffic_light, status);

  Reqd.respond_with_string(req_d, Response.ok(), status);
};

let kill_server = (close_server, req_d, uri) => {
  Log.info("%s  [POST %s]", Emoji.skull_and_crossbones, Uri.to_string(uri));

  Reqd.respond_with_string(req_d, Response.ok(), "killed");

  close_server();
};

let route_mapper = (compiler, close_server) =>
  fun
  | (`PUT, "module") => Some(put_module(compiler))
  | (`POST, "module") => Some(get_module(compiler))
  | (`DELETE, "module") => Some(invalidate_module(compiler))
  | (`DELETE, "context") => Some(invalidate_context(compiler))
  | (`GET, "status") => Some(get_status(compiler))
  | (`POST, "kill") => Some(kill_server(close_server))
  | _ => None;
