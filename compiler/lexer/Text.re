open Core;

let matchers =
  Matcher.[
    bounded(
      "\"",
      s => String(String.sub(s, 1, String.length(s) - 2)) |> result,
      UnclosedString,
    ),
  ];
