open Kore;

exception SourceNotAvailable;

let print = (~buffer_lines=2, contents: string, range: Range.t) => {
  let buffer = Buffer.create(100);
  let lines = contents |> String.split_on_char('\n');
  let start = Range.get_start(range);
  let end_ = Range.get_end(range);

  let first_line = max(Point.get_line(start) - buffer_lines, 0);
  let last_line = Point.get_line(end_) + buffer_lines;
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
        let is_highlight =
          row >= Point.get_line(start) && row <= Point.get_line(end_);

        Fmt.str(
          " %a %a %a\n",
          (is_highlight ? Fmt.red : Fmt.grey)(ppf =>
            Fmt.pf(ppf, "%*d", line_number_width)
          ),
          row,
          Fmt.grey_str,
          "│",
          is_highlight ? Fmt.string : Fmt.grey_str,
          line,
        )
        |> Buffer.add_string(buffer);

        if (is_highlight) {
          let is_start_line = row == Point.get_line(start);
          let is_end_line = row == Point.get_line(end_);
          let line_length = line |> String.length;
          let unhighlighted_prefix_length =
            is_start_line ? Point.get_column(start) - 1 : 0;
          let unhighlighted_suffix_length =
            is_end_line ? line_length - Point.get_column(end_) : 0;

          Buffer.add_string(
            buffer,
            Fmt.str(
              " %*s %a %*s%a\n",
              line_number_width,
              "",
              Fmt.grey_str,
              "│",
              unhighlighted_prefix_length,
              "",
              Fmt.red_str,
              String.repeat(
                line_length
                - unhighlighted_prefix_length
                - unhighlighted_suffix_length,
                "^",
              ),
            ),
          );
        };

        loop(row + 1);

      | None => row < Point.get_line(end_) ? raise(SourceNotAvailable) : ()
      };
    };

  loop(1);

  Buffer.contents(buffer);
};
