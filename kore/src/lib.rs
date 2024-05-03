// #[cfg(feature = "color")]
pub mod color;
#[cfg(feature = "format")]
pub mod format;
mod incrementor;
mod string;

pub use incrementor::Incrementor;
#[cfg(feature = "test")]
pub use pretty_assertions::{assert_eq, assert_ne, assert_str_eq};
#[cfg(feature = "test")]
pub use pretty_assertions_sorted::assert_eq_sorted;
use std::{
    fmt::Display,
    path::{Path, PathBuf},
};

pub trait Generator: Copy {
    type Input;
    type Output: Display;

    fn generate(&self, path: &Path, input: Self::Input) -> (PathBuf, Self::Output);
}

/// represents a state that should not be possible based on the design of the system
#[macro_export]
macro_rules! invariant {
    () => {
        panicking::panic("invariant")
    };
    ($($arg:tt)+) => {
        panic!("invariant: {}", format_args!($($arg)+))
    };
}
