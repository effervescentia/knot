open Kore;

exception SourceNotAvailable;

let print =
    (~buffer_lines=2, ~color=false, contents: string, cursor: Cursor.t) => {
  let buffer = Buffer.create(100);
  let lines = contents |> String.split_on_char('\n');
  let (start, end_) = Cursor.expand(cursor);

  let first_line = max(start.line - buffer_lines, 0);
  let last_line = end_.line + buffer_lines;
  let line_number_width = last_line |> string_of_int |> String.length;

  let rec loop = row =>
    if (row < first_line) {
      if (row > List.length(lines)) {
        raise(SourceNotAvailable);
      };

      loop(row + 1);
    } else if (row <= last_line) {
      switch (List.nth_opt(lines, row - 1)) {
      | Some(line) =>
        let is_highlight = row >= start.line && row <= end_.line;

        Print.fmt(
          " %s │ %s\n",
          Print.fmt("%*d", line_number_width, row)
          |> (color && is_highlight ? Print.red : Functional.identity),
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

      | None => row < end_.line ? raise(SourceNotAvailable) : ()
      };
    };

  loop(1);

  buffer |> Buffer.contents;
};
