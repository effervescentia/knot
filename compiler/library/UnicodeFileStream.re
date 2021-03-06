open Globals;

let of_channel = channel => {
  let read_char = UnicodeFileReader.of_channel(channel);

  LazyStream.of_function(() =>
    switch (read_char()) {
    /* found a newline */
    | Newline(cursor) => Some((Uchar.of_char('\n'), cursor))
    /* found a character */
    | Character(uch, cursor) => Some((uch, cursor))
    /* return None when reaching the end of the file */
    | EndOfFile => None
    }
  );
};
