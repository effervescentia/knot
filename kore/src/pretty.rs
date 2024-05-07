use crate::color::Highlight;
use colored::ColoredString;
use std::path::Path;

pub trait Pretty {
    fn pretty(&self) -> ColoredString;
}

impl<T> Pretty for T
where
    T: AsRef<Path>,
{
    fn pretty(&self) -> ColoredString {
        self.as_ref().to_string_lossy().to_string().highlight()
    }
}
