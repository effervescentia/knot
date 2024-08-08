use crate::Link;
use kore::color::{Colorize, Highlight};
use lang::Range;
use std::fmt::Display;

const CODE_PADDING: usize = 2;

const BORDER: &str = "\u{2502}";
const CORNER: &str = "\u{256d}\u{2500}";

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct CodeFrame<'a> {
    link: &'a Link,
    range: Range,
    source: &'a str,
}

impl<'a> CodeFrame<'a> {
    pub const fn new(link: &'a Link, range: Range, source: &'a str) -> Self {
        Self {
            link,
            range,
            source,
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
        fn format_caret(gutter_width: usize, Range(start, end): Range) -> String {
            let mut caret = format!("{}{} ", " ".repeat(gutter_width), BORDER.subtle());

            for _ in 0..start.1 - 1 {
                caret.push(' ');
            }

            for _ in start.1 - 1..end.1 {
                caret.push('^');
            }

            caret
        }

        fn format_header(gutter_width: usize, link: &Link) -> String {
            let gutter = " ".repeat(gutter_width);

            format!(
                "{gutter}{} {}\n{}",
                CORNER.subtle(),
                link.to_string().highlight(),
                format!("{gutter}{BORDER}").subtle()
            )
        }

        fn format_line(gutter_width: usize, line: usize, code: &str) -> String {
            format!(
                "{:>gutter_width$}{} {}",
                line.to_string().subtle(),
                BORDER.subtle(),
                code.dimmed(),
            )
        }

        let Lines {
            gutter,
            last_row,
            lines,
        } = self.get_lines();

        writeln!(f, "{}", format_header(gutter, self.link))?;

        for (row, line) in lines {
            write!(f, "{}", format_line(gutter, row, line))?;

            if row == self.range.0 .0 {
                write!(f, "\n{}", format_caret(gutter, self.range).error())?;
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
