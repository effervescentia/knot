use crate::Link;
use kore::color::{ClearIf, Colorize, Highlight};
use lang::Range;
use std::fmt::Display;

const CODE_PADDING: usize = 2;

const BORDER: &str = "\u{2502}";
const CORNER: &str = "\u{256d}\u{2500}";

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct CodeFrame<'a> {
    pub root_dir: &'a str,
    pub link: &'a Link,
    pub source: &'a str,
    pub color: bool,
    pub range: Range,
}

impl<'a> CodeFrame<'a> {
    pub const fn link(&self) -> &Link {
        self.link
    }

    fn get_lines(&self) -> Lines {
        let Range(start, end) = self.range;

        let sample_start = start.0.checked_sub(CODE_PADDING).unwrap_or_default();
        let sample_end = end.0 + CODE_PADDING + 1;

        let mut lines = vec![];
        let mut gutter = 1;
        let mut last_row = end.0;

        for row in sample_start..sample_end {
            if let Some(line) = row
                .checked_sub(1)
                .and_then(|index| self.source.lines().nth(index))
            {
                // ignore preceding empty lines
                if lines.is_empty() && line.is_empty() {
                    continue;
                }

                lines.push((row, line));
                gutter = row.to_string().len().max(gutter);
                last_row = row;
            }
        }

        // trim trailing empty lines
        while let Some((_, line)) = lines.last() {
            if line.is_empty() {
                lines.pop();
                last_row -= 1;
            } else {
                break;
            }
        }

        Lines {
            gutter,
            last_row,
            lines,
        }
    }

    fn format_header(&self, gutter_width: usize, no_color: bool) -> String {
        let gutter = " ".repeat(gutter_width);
        let link = self.link();

        format!(
            "{gutter}{} {} {}\n{}\n",
            CORNER.subtle().clear_if(no_color),
            link.to_string().highlight().clear_if(no_color),
            format!(
                "({root_dir}/{link}:{point})",
                root_dir = self.root_dir,
                point = self.range.0
            )
            .subtle()
            .clear_if(no_color),
            format!("{gutter}{BORDER}").subtle().clear_if(no_color)
        )
    }
}

impl<'a> Display for CodeFrame<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fn format_single_caret(
            gutter_width: usize,
            line_width: usize,
            row: usize,
            Range(start, end): Range,
            no_color: bool,
        ) -> String {
            let is_wrapping = start.0 != end.0;
            let mut caret = String::new();

            let mut fill_range = |from, to| {
                for _ in 0..from {
                    caret.push(' ');
                }
                for _ in from..to {
                    caret.push('^');
                }
            };

            if is_wrapping {
                if row == start.0 {
                    fill_range(start.1 - 1, line_width);
                } else if row < end.0 {
                    fill_range(0, line_width);
                } else if row == end.0 {
                    fill_range(0, end.1);
                }
            } else {
                fill_range(start.1 - 1, end.1);
            }

            format!(
                "{}{} {}",
                " ".repeat(gutter_width),
                BORDER.subtle().clear_if(no_color),
                caret.error().clear_if(no_color)
            )
        }

        fn format_dual_caret(
            gutter_width: usize,
            line_width: usize,
            row: usize,
            Range(start_highlight, end_highlight): Range,
            Range(start_error, end_error): Range,
            no_color: bool,
        ) -> String {
            let mut highlight_caret = String::new();
            let mut error_caret = String::new();

            let fill_range = |caret: &mut String, from, to| {
                for _ in 0..from {
                    caret.push(' ');
                }
                for _ in from..to {
                    caret.push('^');
                }
            };

            let is_highlight_wrapping = start_highlight.0 != end_highlight.0;
            if is_highlight_wrapping {
                if row == start_highlight.0 {
                    fill_range(&mut highlight_caret, start_highlight.1 - 1, line_width);
                } else if row < end_highlight.0 {
                    fill_range(&mut highlight_caret, 0, line_width);
                } else if row == end_highlight.0 {
                    fill_range(&mut highlight_caret, 0, end_highlight.1);
                }
            } else {
                fill_range(&mut highlight_caret, start_error.1 - 1, end_error.1);
            }

            let is_error_wrapping = start_error.0 != end_error.0;
            if is_error_wrapping {
                if row == start_error.0 {
                    fill_range(&mut error_caret, start_error.1 - 1, line_width);
                } else if row < end_error.0 {
                    fill_range(&mut error_caret, 0, line_width);
                } else if row == end_error.0 {
                    fill_range(&mut error_caret, 0, end_error.1);
                }
            } else {
                fill_range(&mut error_caret, start_error.1 - 1, end_error.1);
            }

            format!(
                "{}{} {}{}",
                " ".repeat(gutter_width),
                BORDER.subtle().clear_if(no_color),
                highlight_caret.highlight().clear_if(no_color),
                error_caret.error().clear_if(no_color)
            )
        }

        fn format_line(gutter_width: usize, line: usize, code: &str, no_color: bool) -> String {
            format!(
                "{:>gutter_width$}{} {}",
                line.to_string().subtle().clear_if(no_color),
                BORDER.subtle().clear_if(no_color),
                code.dimmed().clear_if(no_color),
            )
        }

        let no_color = !self.color;
        let Lines {
            gutter,
            last_row,
            lines,
        } = self.get_lines();

        self.format_header(gutter, no_color).fmt(f)?;

        for (row, line) in lines {
            format_line(gutter, row, line, no_color).fmt(f)?;

            if row >= self.range.0 .0 && row <= self.range.1 .0 {
                write!(
                    f,
                    "\n{}",
                    format_single_caret(gutter, line.len(), row, self.range, no_color)
                )?;
            }

            if row != last_row {
                writeln!(f)?;
            }
        }

        Ok(())
    }
}

struct Lines<'a> {
    gutter: usize,
    last_row: usize,
    lines: Vec<(usize, &'a str)>,
}

#[cfg(test)]
mod tests {
    use super::CodeFrame;
    use crate::Link;
    use kore::assert_str_eq;
    use lang::Range;

    #[test]
    fn highlight_range() {
        let link = Link::mock();
        let source = "const FOO = 123;
const BAR = FOO + 10;
type Fizz = integer;
type Buzz = boolean;";

        assert_str_eq!(
            CodeFrame {
                root_dir: "./src",
                link: &link,
                source,
                range: Range::new((2, 13), (2, 15)),
                color: false
            }
            .to_string(),
            " \u{256d}\u{2500} mock.kn
 \u{2502}
1\u{2502} const FOO = 123;
2\u{2502} const BAR = FOO + 10;
 \u{2502}             ^^^
3\u{2502} type Fizz = integer;
4\u{2502} type Buzz = boolean;"
        );
    }

    #[test]
    fn trim_empty_lines() {
        let link = Link::mock();
        let source = "

const FOO = 123;

";

        assert_str_eq!(
            CodeFrame {
                root_dir: "./src",
                link: &link,
                source,
                range: Range::new((3, 13), (3, 15)),
                color: false
            }
            .to_string(),
            " \u{256d}\u{2500} mock.kn
 \u{2502}
3\u{2502} const FOO = 123;
 \u{2502}             ^^^"
        );
    }
}
