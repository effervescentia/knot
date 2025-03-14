// #[cfg(feature = "color")]
pub mod color;
#[cfg(feature = "format")]
pub mod format;
pub mod graph;
mod incrementor;
pub mod internal;
pub mod pretty;
mod string;
pub mod uniqueue;

pub use incrementor::Incrementor;
#[cfg(feature = "test")]
pub use pretty_assertions::{assert_eq, assert_ne, assert_str_eq};
#[cfg(feature = "test")]
pub use pretty_assertions_sorted::assert_eq_sorted;

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
