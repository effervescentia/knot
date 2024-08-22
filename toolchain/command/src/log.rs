use kore::{color::Highlight, pretty::Pretty};
use std::path::Path;

pub fn entrypoint(verbose: bool, entry: &Path) {
    if verbose {
        eprintln!(
            "\u{1f4cd} starting from entrypoint {}",
            entry.pretty().focus()
        );
    }
}

pub fn glob(verbose: bool, pattern: &str) {
    if verbose {
        eprintln!("\u{1f4cd} scanning for files matching {}", pattern.focus());
    }
}

pub fn success(verbose: bool, status: &str, count: usize) {
    if verbose {
        eprintln!();
    }

    eprintln!(
        "{} {} {}\n",
        status.success(),
        count.to_string().focus(),
        "module(s) with no errors".success()
    );
}
