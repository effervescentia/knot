open Httpaf;
open Kore;
open Kore.Compiler;

module Response = Server_Response;
module Generator = KnotGenerate.Generator;
module CompilerUtil = KnotCompile.Util;

let log_incoming = (~addon=?, ~emoji=Emoji.chequered_flag, req_d, uri) =>
  Log.info(
    "%s  [%s %s]%s",
    emoji,
    Reqd.request(req_d).meth |> Method.to_string,
    Uri.to_string(uri),
    switch (addon) {
    | Some(str) => Printf.sprintf(" (%s)", str)
    | None => ""
    },
  );

let read_to_string = (req_d, f) => {
  let req_body = Reqd.request_body(req_d);
  let str = ref("");

  let rec read = () =>
    Body.schedule_read(
      req_body,
      ~on_eof=() => f(str^),
      ~on_read=
        (data, ~off, ~len) => {
          let str_part = Bigstringaf.substring(~off, ~len, data);
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
      log_incoming(~addon=module_path, req_d, uri);

      switch (compiler.add(module_path)) {
      | _ =>
        Reqd.respond_with_string(
          req_d,
          Response.ok(),
          "module added to context",
        )
      | exception exn =>
        switch (exn) {
        | exn =>
          Printexc.to_string(exn) |> Log.error("module failed: %s");
          Printexc.print_backtrace(stdout);
        };

        Reqd.respond_with_string(
          req_d,
          Response.not_found(),
          Printf.sprintf("failed to add %s to context", module_path),
        );
      };
    },
  );

let get_module = ({source_dir}, compiler, req_d, uri) =>
  read_to_string(
    req_d,
    module_path => {
      log_incoming(~addon=module_path, req_d, uri);

      compiler.find(module_path)
      |> (
        fun
        | Some(ast) => {
            let res_body =
              Response.ok() |> Reqd.respond_with_streaming(req_d);

            Generator.generate(
              str =>
                Bigstringaf.of_string(~off=0, ~len=String.length(str), str)
                |> Body.write_bigstring(res_body),
              Util.normalize_module(source_dir),
              ast,
            );

            Body.close_writer(res_body);
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
      log_incoming(~addon=module_path, ~emoji=Emoji.cross_mark, req_d, uri);

      compiler.invalidate(module_path);

      Reqd.respond_with_string(req_d, Response.ok(), "module invalidated");

      compiler.add(module_path);
    },
  );

let invalidate_context = (compiler, req_d, uri) => {
  log_incoming(~emoji=Emoji.skull_and_crossbones, req_d, uri);

  compiler.reset();

  Reqd.respond_with_string(req_d, Response.ok(), "build context invalidated");
};

let get_status = (compiler, req_d, uri) => {
  log_incoming(req_d, uri);

  let status =
    switch (compiler.status()) {
    | Idle => "idle"
    | Running => "running"
    | Complete => "complete"
    };

  Log.info("%s  [%s]", Emoji.vertical_traffic_light, status);

  Reqd.respond_with_string(req_d, Response.ok(), status);
};

let get_module_status = (compiler, req_d, uri) =>
  read_to_string(
    req_d,
    module_path => {
      log_incoming(~addon=module_path, req_d, uri);

      let status =
        try (
          compiler.find(module_path)
          |> (
            fun
            | Some(_) => "complete"
            | None => "pending"
          )
        ) {
        | _ => "failed"
        };

      /* compiler.debug_modules(); */

      Log.info(
        "%s  [%s] (%s)",
        Emoji.vertical_traffic_light,
        status,
        module_path,
      );

      Reqd.respond_with_string(req_d, Response.ok(), status);
    },
  );

let kill_server = (close_server, req_d, uri) => {
  log_incoming(~emoji=Emoji.skull_and_crossbones, req_d, uri);

  Reqd.respond_with_string(req_d, Response.ok(), "killed");

  close_server();
};

let route_mapper = (paths, compiler, close_server) =>
  fun
  | (`PUT, "module") => Some(put_module(compiler))
  | (`POST, "module") => Some(get_module(paths, compiler))
  | (`DELETE, "module") => Some(invalidate_module(compiler))
  | (`POST, "module/status") => Some(get_module_status(compiler))
  | (`DELETE, "context") => Some(invalidate_context(compiler))
  | (`GET, "status") => Some(get_status(compiler))
  | (`POST, "kill") => Some(kill_server(close_server))
  | _ => None;
