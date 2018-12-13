exception InvalidLineEncoding;

let print_ch = ch =>
  Printf.sprintf("CHAR: '%c'", ch) |> String.escaped |> print_string;
let print_cursor = ((col, row)) =>
  Printf.sprintf(" on [%d:%d]", col, row) |> print_endline;

let of_channel = channel => {
  let row = ref(1);
  let column = ref(1);

  let create_cursor = () => (row^, column^);

  let rec next = () =>
    switch (input_char(channel)) {
    | '\r' =>
      switch (next()) {
      | ('\n' as res, _) => (res, create_cursor())
      | _ => raise(InvalidLineEncoding)
      }
    | res => (res, create_cursor())
    };

  LazyStream.of_function(() =>
    switch (next()) {
    | ('\n' as ch, cursor) as res =>
      print_ch(ch);
      print_cursor(cursor);
      row := row^ + 1;
      column := 1;
      Some(res);
    | (ch, cursor) as res =>
      print_ch(ch);
      print_cursor(cursor);
      column := column^ + 1;
      Some(res);

    | exception End_of_file => None
    }
  );
};