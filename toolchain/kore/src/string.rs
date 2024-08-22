/// represents a state that should not be possible based on the design of the system
#[macro_export]
macro_rules! str {
    ($arg:literal) => {
        String::from($arg)
    };
}
