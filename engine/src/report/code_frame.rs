use crate::Link;
use kore::color::{ClearIf, Colorize, Highlight};
use lang::{Point, Range};
use std::fmt::Display;

const CODE_PADDING: usize = 2;

const BORDER: &str = "\u{2502}";
const CORNER: &str = "\u{256d}\u{2500}";

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct CodeFrame<'a> {
    root_dir: &'a str,
    link: &'a Link,
    range: Range,
    source: &'a str,
    color: bool,
}

impl<'a> CodeFrame<'a> {
    pub const fn color(root_dir: &'a str, link: &'a Link, source: &'a str, range: Range) -> Self {
        Self {
            root_dir,
            link,
            range,
            source,
            color: true,
        }
    }

    pub const fn no_color(
        root_dir: &'a str,
        link: &'a Link,
        source: &'a str,
        range: Range,
    ) -> Self {
        Self {
            root_dir,
            link,
            range,
            source,
            color: false,
        }
    }

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
}

impl<'a> Display for CodeFrame<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fn format_caret(gutter_width: usize, Range(start, end): Range, no_color: bool) -> String {
            let mut caret = String::new();

            for _ in 0..start.1 - 1 {
                caret.push(' ');
            }

            for _ in start.1 - 1..end.1 {
                caret.push('^');
            }

            format!(
                "{}{} {}",
                " ".repeat(gutter_width),
                BORDER.subtle().clear_if(no_color),
                caret.error().clear_if(no_color)
            )
        }

        fn format_header(
            gutter_width: usize,
            root_dir: &str,
            link: &Link,
            point: Point,
            no_color: bool,
        ) -> String {
            let gutter = " ".repeat(gutter_width);

            format!(
                "{gutter}{} {} {}\n{}",
                CORNER.subtle().clear_if(no_color),
                link.to_string().highlight().clear_if(no_color),
                format!("({root_dir}/{link}:{point})")
                    .subtle()
                    .clear_if(no_color),
                format!("{gutter}{BORDER}").subtle().clear_if(no_color)
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

        writeln!(
            f,
            "{}",
            format_header(gutter, self.root_dir, self.link, self.range.0, no_color)
        )?;

        for (row, line) in lines {
            write!(f, "{}", format_line(gutter, row, line, no_color))?;

            if row == self.range.0 .0 {
                write!(f, "\n{}", format_caret(gutter, self.range, no_color))?;
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
            CodeFrame::no_color("./src", &link, source, Range::new((2, 13), (2, 15))).to_string(),
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
            CodeFrame::no_color("./src", &link, source, Range::new((3, 13), (3, 15))).to_string(),
            " \u{256d}\u{2500} mock.kn
 \u{2502}
3\u{2502} const FOO = 123;
 \u{2502}             ^^^"
        );
    }
}
