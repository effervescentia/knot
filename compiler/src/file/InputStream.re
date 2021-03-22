open Kore;

let _to_cursor = (cursor, decoder) =>
  cursor
    ? Cursor.point(Uutf.decoder_line(decoder), Uutf.decoder_col(decoder))
    : Cursor.zero;

let _to_stream = (cursor, decoder) => {
  let next = _ =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uchar) =>
      Some(Input.create(uchar, _to_cursor(cursor, decoder)))
    | `Malformed(uchar) =>
      Some(Input.create(Uutf.u_rep, _to_cursor(cursor, decoder)))
    | `End => None
    | `Await => assert(false)
    };

  Stream.from(next);
};

let _normalize = `Readline(Uchar.of_char(Constants.Character.eol));

/**
 wrapper to ingest a stream of unicode characters
 */
type t = Stream.t(Input.t);

/* static */

let of_string = (~cursor=true, s: string): t =>
  Uutf.decoder(~nln=_normalize, `String(s)) |> _to_stream(cursor);

let of_channel = (~cursor=true, channel: in_channel): t =>
  Uutf.decoder(~nln=_normalize, `Channel(channel)) |> _to_stream(cursor);
