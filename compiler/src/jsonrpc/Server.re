open Kore;

module Event = Protocol.Event;
module Writer = Protocol.Writer;
module Reader = Protocol.Reader;

type t = {
  send: JSON.t => unit,
  reply: (int, Event.result_t) => unit,
  notify: (string, JSON.t) => unit,
  watch: (Event.t => unit) => Lwt.t(unit),
};

/* static */

let create = (in_: in_channel, out: out_channel): t => {
  let stream = Stream.of_channel(in_);

  let send = Writer.write_to_channel(out);

  let notify = (method, params) =>
    Protocol.notification(method, params) |> send;

  let reply = id =>
    fun
    | Ok(result) => Protocol.response(id, result) |> send
    | Error(report) => report(id) |> send;

  let watch = handler =>
    Lwt.wrap(() => {
      Log.info(
        "watching for %s requests via %s",
        "JSONRPC" |> ~@Fmt.good_str,
        "stdin" |> ~@Fmt.good_str,
      );

      while (true) {
        try(stream |> Reader.read_from_stream |> handler) {
        | Protocol.BuiltinError((ParseError | InvalidRequest) as err) =>
          err |> Protocol.builtin_error |> send
        };
      };
    });

  {send, notify, reply, watch};
};
