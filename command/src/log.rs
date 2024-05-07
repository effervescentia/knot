use kore::{color::Highlight, pretty::Pretty};
use std::path::Path;

pub fn entrypoint(entry: &Path) {
    eprintln!(
        "\u{1f4cd} starting from entrypoint {}",
        entry.pretty().focus()
    );
}

pub fn glob(pattern: &str) {
    eprintln!("\u{1f4cd} scanning for files matching {}", pattern.focus());
}

pub fn parsed_from_entry(count: usize) {
    eprintln!(
        "\u{1f440} parsed {} module(s) by traversing the import graph",
        count.to_string().focus()
    );
}

pub fn parsed_from_glob(count: usize) {
    eprintln!(
        "\u{1f440} parsed {} matching module(s)",
        count.to_string().focus()
    );
}

pub fn linked() {
    eprintln!("\u{1f517} linked all modules and libraries");
}

pub fn analyzed() {
    eprintln!("\u{2705} analysis complete");
}

pub fn success(status: &str, count: usize) {
    eprintln!(
        "\n{} {} {}\n",
        status.success(),
        count.to_string().focus(),
        "module(s) with no errors".success()
    );
}
