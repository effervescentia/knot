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
    | 0 => buffer |> Buffer.contents
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

  Event.deserialize(method_, json);
};

let watch_events = (stream: Stream.t(char), handler: Event.t => unit) => {
  while (true) {
    read_event(stream) |?> handler |> ignore;
  };
};

let send = (res: Yojson.Basic.t) => {
  res |> Response.serialize |> Print.fprintf(stdout, "%s");
  flush(stdout);
};

let reply = (req: Event.request_t('a), create_res) => {
  let res = create_res(req.id);

  Log.debug(
    "response_to: %d result: %s",
    req.id,
    Yojson.Basic.pretty_to_string(res),
  );

  send(res);
};

let subscribe = (channel: in_channel, handler: Event.t => unit): unit => {
  let stream = channel |> Stream.of_channel;

  Event.(
    switch (read_event(stream)) {
    | Some(Initialize(req) as event) =>
      Log.info("server initialized successfully");

      Response.initialize(
        "knot",
        req.params.capabilities
        |?< (x => x.workspace)
        |?< (x => x.workspace_folders)
        |?: false,
      )
      |> reply(req);

      handler(event);

      watch_events(stream, handler);
    | _ => failwith("did not receive initialize request first")
    }
  );
};
