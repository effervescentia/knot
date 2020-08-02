open Globals;

type t =
  | Newline(cursor)
  | Character(Uchar.t, cursor)
  | EndOfFile;

let _normalized_newline_char_code = 10;

let _get_cursor = decoder => (
  Uutf.decoder_line(decoder),
  Uutf.decoder_col(decoder),
);

let of_channel = channel => {
  let decoder =
    Uutf.decoder(
      ~nln=`Readline(Uchar.of_int(_normalized_newline_char_code)),
      `Channel(channel),
    );

  () =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uch) =>
      let cursor = _get_cursor(decoder);

      if (Uchar.to_int(uch) == _normalized_newline_char_code) {
        Newline(cursor);
      } else {
        Character(uch, cursor);
      };
    | `End => EndOfFile
    | `Malformed(_) => Character(Uutf.u_rep, _get_cursor(decoder))
    /* this will not happen unless providing a manual source */
    | `Await => assert(false)
    };
};
