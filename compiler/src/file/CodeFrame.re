open Kore;

exception SourceNotAvailable(string);

let print = (~buffer_lines=2, ~color=false, file: string, cursor: Cursor.t) => {
  let buffer = Buffer.create(100);
  let (start, end_) = Cursor.expand(cursor);

  let first_line = max(start.line - buffer_lines, 0);
  let last_line = end_.line + buffer_lines;
  let line_number_width = last_line |> string_of_int |> String.length;
  let in_ = open_in(file);

  let rec loop = row =>
    if (row < first_line) {
      try(input_line(in_) |> ignore) {
      | End_of_file => raise(SourceNotAvailable(file))
      };

      loop(row + 1);
    } else if (row <= last_line) {
      switch (input_line(in_)) {
      | line =>
        let is_highlight = row >= start.line && row <= end_.line;

        Print.fmt(
          " %*s │ %s\n",
          line_number_width,
          color && is_highlight
            ? Print.red(row |> string_of_int) : Print.fmt("%d", row),
          line,
        )
        |> Buffer.add_string(buffer);

        if (is_highlight) {
          let is_start_line = row == start.line;
          let is_end_line = row == end_.line;
          let line_length = line |> String.length;
          let unhighlighted_prefix_length =
            is_start_line ? start.column - 1 : 0;
          let unhighlighted_suffix_length =
            is_end_line ? line_length - end_.column : 0;

          Buffer.add_string(
            buffer,
            Print.fmt(
              " %*s │ %*s%s\n",
              line_number_width,
              "",
              unhighlighted_prefix_length,
              "",
              String.repeat(
                line_length
                - unhighlighted_prefix_length
                - unhighlighted_suffix_length,
                "^",
              )
              |> (color ? Print.red : Functional.identity),
            ),
          );
        };

        loop(row + 1);

      | exception End_of_file =>
        row < end_.line ? raise(SourceNotAvailable(file)) : ()
      };
    };

  loop(1);
  close_in(in_);

  buffer |> Buffer.contents;
};
