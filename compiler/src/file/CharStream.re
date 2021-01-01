open Kore;

let _to_cursor = decoder =>
  Cursor.point(Uutf.decoder_line(decoder), Uutf.decoder_col(decoder));

let _to_stream = decoder => {
  let next = _ =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uchar) => Some(Char.make(uchar, _to_cursor(decoder)))
    | `Malformed(uchar) => Some(Char.make(Uutf.u_rep, _to_cursor(decoder)))
    | `End => None
    | `Await => assert(false)
    };

  Stream.from(next);
};

let _normalize = `Readline(Uchar.of_char(Constants.Character.eol));

let of_string = (s: string): Stream.t(Char.t) =>
  Uutf.decoder(~nln=_normalize, `String(s)) |> _to_stream;

let of_channel = (channel: in_channel): Stream.t(Char.t) =>
  Uutf.decoder(~nln=_normalize, `Channel(channel)) |> _to_stream;
