open Kore;

module Event = Protocol.Event;
module Writer = Protocol.Writer;
module Reader = Protocol.Reader;

type t = {
  send: JSON.t => unit,
  reply: (int, Event.result_t) => unit,
  watch: (Event.t => unit) => Lwt.t(unit),
};

/* static */

let create = (in_: in_channel, out: out_channel): t => {
  let stream = Stream.of_channel(in_);

  let send = Writer.write_to_channel(out);

  let reply = id =>
    fun
    | Ok(res) => Protocol.response(id, res) |> send
    | Error((code, message, data)) =>
      Protocol.error(~data, id, code, message) |> send;

  let watch = handler =>
    Lwt.wrap(() => {
      while (true) {
        try(stream |> Reader.read_from_stream |> handler) {
        | Protocol.BuiltinError((ParseError | InvalidRequest) as err) =>
          err |> Protocol.builtin_error |> send
        };
      }
    });

  {send, reply, watch};
};
