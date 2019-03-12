open Globals;

let of_channel = channel => {
  let row = ref(1);
  let column = ref(1);

  let rec next = () => {
    let cursor = (row^, column^);

    switch (input_char(channel)) {
    | '\r' =>
      switch (next()) {
      | ('\n' as res, _) => (res, cursor)
      /* \r must be followed by a \n character in CRLF line encoding */
      | _ => raise(InvalidLineEncoding)
      }
    | res => (res, cursor)
    };
  };

  LazyStream.of_function(() =>
    switch (next()) {
    /* found a newline, increase the row and reset the column */
    | ('\n', _) as res =>
      row := row^ + 1;
      column := 1;
      Some(res);
    /* found a character, increase the column */
    | _ as res =>
      column := column^ + 1;
      Some(res);
    /* return None when reaaching the end of the file */
    | exception End_of_file => None
    }
  );
};
