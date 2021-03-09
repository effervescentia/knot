/**
 Wrapper to ingest a stream of unicode characters.
 */
open Kore;

let _to_cursor = (cursor, decoder) =>
  cursor
    ? Cursor.point(Uutf.decoder_line(decoder), Uutf.decoder_col(decoder))
    : Cursor.zero;

let _to_stream = (cursor, decoder) => {
  let next = _ =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uchar) =>
      Some(Char.create(uchar, _to_cursor(cursor, decoder)))
    | `Malformed(uchar) =>
      Some(Char.create(Uutf.u_rep, _to_cursor(cursor, decoder)))
    | `End => None
    | `Await => assert(false)
    };

  Stream.from(next);
};

let _normalize = `Readline(Uchar.of_char(Constants.Character.eol));

let of_string = (~cursor=true, s: string): Stream.t(Char.t) =>
  Uutf.decoder(~nln=_normalize, `String(s)) |> _to_stream(cursor);

let of_channel = (~cursor=true, channel: in_channel): Stream.t(Char.t) =>
  Uutf.decoder(~nln=_normalize, `Channel(channel)) |> _to_stream(cursor);
