open Httpaf;

module Response = Server_Response;

let put_module = (compiler, req_d, uri) => {
  Log.info("%s  [PUT] %s", Emoji.inbox_tray, Uri.to_string(uri));

  let req_body = Reqd.request_body(req_d);
  let res_body = Response.ok() |> Reqd.respond_with_streaming(req_d);

  let rec respond = () =>
    Body.schedule_read(
      req_body,
      ~on_eof=() => Body.close_writer(res_body),
      ~on_read=
        (data, ~off, ~len) => {
          Body.write_bigstring(res_body, data, ~off, ~len);

          /* continue to write until body is empty */
          respond();
        },
    );

  respond();
};

let get_module = (compiler, req_d, uri) => {
  Log.info("%s  [GET] %s", Emoji.outbox_tray, Uri.to_string(uri));

  let req_body = Reqd.request_body(req_d);
  let res_body = Response.ok() |> Reqd.respond_with_streaming(req_d);

  let rec respond = () =>
    Body.schedule_read(
      req_body,
      ~on_eof=() => Body.close_writer(res_body),
      ~on_read=
        (data, ~off, ~len) => {
          Body.write_bigstring(res_body, data, ~off, ~len);

          /* continue to write until body is empty */
          respond();
        },
    );

  respond();
};

let invalidate_module = (compiler, req_d, _) =>
  Reqd.respond_with_string(req_d, Response.ok(), "module invalidated");

let invalidate_context = (compiler, req_d, _) =>
  Reqd.respond_with_string(req_d, Response.ok(), "build context invalidated");

let get_status = (req_d, uri) => {
  Log.info("%s  [GET] %s", Emoji.outbox_tray, Uri.to_string(uri));

  Reqd.respond_with_string(req_d, Response.ok(), "running");
};
let route_mapper = compiler =>
  fun
  | (`PUT, "module") => Some(put_module(compiler))
  | (`GET, "module") => Some(get_module(compiler))
  | (`DELETE, "module") => Some(invalidate_module(compiler))
  | (`DELETE, "context") => Some(invalidate_context(compiler))
  | (`GET, "status") => Some(get_status)
  | _ => None;
