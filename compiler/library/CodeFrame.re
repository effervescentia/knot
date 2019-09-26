let _buffer_size = 512;
let _padding_width = 4;

let _add_line_number = (buf, margin_size, row) => {
  let line_number = string_of_int(row);

  Printf.sprintf(
    "%s%s",
    ANSITerminal.(sprintf([blue], "%s", line_number)),
    String.make(margin_size - String.length(line_number), ' '),
  )
  |> Buffer.add_string(buf);
};

let print = (file, cursor) => {
  let in_channel = open_in(file);
  let read_char = UnicodeFileReader.of_channel(in_channel);
  let start_row = max(1, fst(cursor) - _padding_width);
  let end_row = fst(cursor) + _padding_width;
  let margin_size = (string_of_int(end_row) |> String.length) + 1;

  if (start_row != 1) {
    let rec skip = () =>
      switch (read_char()) {
      | Newline((curr_row, _)) when curr_row == start_row => ()
      | _ => skip()
      };
    skip();
  };

  let buf = Buffer.create(_buffer_size);
  let add_line_number = _add_line_number(buf, margin_size);

  Buffer.add_char(buf, '\n');
  add_line_number(start_row);

  let indicator_added = ref(false);
  let add_indicator = () => {
    indicator_added := true;

    ANSITerminal.(
      Printf.sprintf(
        "%s%s\n",
        String.make(snd(cursor) - 1 + margin_size, ' '),
        sprintf([red, Bold], "^"),
      )
    )
    |> Buffer.add_string(buf);
  };

  let rec loop = () =>
    switch (read_char()) {
    | Newline((row, _)) when row == fst(cursor) + _padding_width + 1 => ()
    | Newline(ch_cursor) =>
      Buffer.add_char(buf, '\n');

      if (! indicator_added^ && fst(ch_cursor) == fst(cursor) + 1) {
        add_indicator();
      };

      add_line_number(fst(ch_cursor));

      loop();
    | Character(ch, (row, col)) =>
      Buffer.add_utf_8_uchar(buf, ch);
      loop();
    | EndOfFile =>
      if (! indicator_added^) {
        add_indicator();
      }
    };
  loop();

  close_in(in_channel);
  Buffer.add_char(buf, '\n');

  Buffer.contents(buf);
};
