open Kore;

let __content_length_header = "Content-Length";

let rec read_headers = stream => {
  let rec loop = buffer => {
    let buffer = buffer ++ (Stream.next(stream) |> String.make(1));

    if (String.ends_with("\r\n", buffer)) {
      buffer |> String.drop_all_suffix("\r\n");
    } else {
      loop(buffer);
    };
  };

  switch (Stream.npeek(2, stream)) {
  | ['\r', '\n'] => []
  | _ =>
    let header = loop("") |> String.split(": ");

    [header, ...read_headers(stream)];
  };
};

let read_content = (length, stream) => {
  let buffer = Buffer.create(length);

  let rec loop =
    fun
    | 0 => Buffer.contents(buffer)
    | remaining => {
        Buffer.add_char(buffer, Stream.next(stream));
        loop(remaining - 1);
      };

  loop(length);
};

let read_event = (stream: Stream.t(char)) => {
  let headers = read_headers(stream);

  Stream.junk(stream);
  Stream.junk(stream);

  let content_length =
    headers
    |> List.fold_left(
         (acc, (key, value)) =>
           key == __content_length_header ? int_of_string(value) : acc,
         0,
       );

  let content = stream |> read_content(content_length);

  let json = Yojson.Basic.from_string(content);
  let method_ = json |> Yojson.Basic.Util.(member("method") % to_string);

  Log.debug(
    "event: %s data: %s",
    method_,
    Yojson.Basic.pretty_to_string(json),
  );

  (method_, json);
};

let watch_events =
    (stream: Stream.t(char), handler: ((string, Yojson.Basic.t)) => unit) => {
  while (true) {
    read_event(stream) |> handler;
  };
};

let send = (res: Yojson.Basic.t) => {
  res |> Response.serialize |> Printf.fprintf(stdout, "%s");
  flush(stdout);
};

let reply = (req: Deserialize.request_t('a), create_res) => {
  let res = create_res(req.id);

  Log.debug(
    "response_to: %d result: %s",
    req.id,
    Yojson.Basic.pretty_to_string(res),
  );

  send(res);
};

let notify = (notification: Yojson.Basic.t) => {
  Log.debug("notification: %s", Yojson.Basic.pretty_to_string(notification));

  send(notification);
};

let report = errs =>
  send(
    Response.show_message(
      Fmt.str("compilation failed with errors:\n%a", pp_err_list, errs),
      Error,
    ),
  );
