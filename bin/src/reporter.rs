use command::Error;
use kore::color::{Colorize, Highlight};

const ERROR_HEADER: &str = "\u{2554}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2557}
\u{2551}                    FAILED                    \u{2551}
\u{255a}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{2550}\u{255d}";

pub fn eprint_report(errors: &[Error]) {
    let error_count = errors.len();
    let error_count_bumper =
        format!("finished with {} error(s)", error_count.to_string().bold()).error();

    eprintln!("{}\n\n{}\n", ERROR_HEADER.error(), error_count_bumper);

    for (index, error) in errors.iter().enumerate() {
        eprintln!(
            "{index} {error}\n",
            index = format!("{})", index + 1).error()
        );
    }

    eprintln!("{}\n", error_count_bumper);
}
