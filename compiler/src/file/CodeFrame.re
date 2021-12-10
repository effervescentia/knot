open Kore;

exception SourceNotAvailable;

let pp = (~buffer_lines=2): Fmt.t((string, Range.t)) =>
  (ppf, (contents, range)) => {
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

          Fmt.(
            pf(
              ppf,
              " %a %a %a\n",
              (is_highlight ? red : grey)(ppf =>
                pf(ppf, "%*d", line_number_width)
              ),
              row,
              grey_str,
              "│",
              is_highlight ? string : grey_str,
              line,
            )
          );

          if (is_highlight) {
            let is_start_line = row == Point.get_line(start);
            let is_end_line = row == Point.get_line(end_);
            let line_length = line |> String.length;
            let unhighlighted_prefix_length =
              is_start_line ? Point.get_column(start) - 1 : 0;
            let unhighlighted_suffix_length =
              is_end_line ? line_length - Point.get_column(end_) : 0;
            let highlight_length =
              line_length
              - unhighlighted_prefix_length
              - unhighlighted_suffix_length;

            Fmt.(
              pf(
                ppf,
                " %*s %a %*s%a\n",
                line_number_width,
                "",
                grey_str,
                "│",
                unhighlighted_prefix_length,
                "",
                red_str,
                String.repeat(highlight_length, "^"),
              )
            );
          };

          loop(row + 1);

        | None => row < Point.get_line(end_) ? raise(SourceNotAvailable) : ()
        };
      };

    loop(1);
  };
