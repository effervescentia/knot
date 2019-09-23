open Globals;

type t =
  | Newline((int, int))
  | Character(Uchar.t, (int, int))
  | EndOfFile;

let normalized_newline_char_code = 10;

let of_channel = channel => {
  let decoder =
    Uutf.decoder(
      ~nln=`Readline(Uchar.of_int(normalized_newline_char_code)),
      `Channel(channel),
    );

  let get_cursor = () => (
    Uutf.decoder_line(decoder),
    Uutf.decoder_col(decoder),
  );

  () =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uch) =>
      let cursor = get_cursor();
      switch (Uchar.to_int(uch)) {
      | x when x == normalized_newline_char_code => Newline(cursor)
      | _ => Character(uch, cursor)
      };
    | `End => EndOfFile
    | `Malformed(_) => Character(Uutf.u_rep, get_cursor())
    /* this will not happen unless providing a manual source */
    | `Await => assert(false)
    };
};
