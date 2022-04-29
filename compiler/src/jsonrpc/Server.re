open Kore;

module Writer = Protocol.Writer;
module Reader = Protocol.Reader;

type t = {
  send: JSON.t => unit,
  watch: (Protocol.request_t => unit) => Lwt.t(unit),
};

/* static */

let create = (in_: in_channel, out: out_channel): t => {
  let stream = Stream.of_channel(in_);

  let send = Writer.write_to_channel(out);

  let watch = handler =>
    Lwt.wrap(() => {
      while (true) {
        try(stream |> Reader.read_from_stream |> handler) {
        | Protocol.BuiltinError((ParseError | InvalidRequest) as err) =>
          err |> Protocol.builtin_error |> send
        };
      }
    });

  {send, watch};
};
