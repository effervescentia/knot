use std::fmt::{Display, Formatter, Write};

pub fn indented<T>(f: &mut T) -> indenter::Indented<T>
where
    T: Sized,
{
    indenter::indented(f).with_str("  ")
}

struct Parameters<'a, T>(&'a Vec<T>)
where
    T: Display;

impl<'a, T> Display for Parameters<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if self.0.is_empty() {
            Ok(())
        } else {
            write!(f, "({})", SeparateEach(", ", self.0))
        }
    }
}

pub struct SeparateEach<'a, T>(pub &'a str, pub &'a Vec<T>)
where
    T: Display;

impl<'a, T> Display for SeparateEach<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        let mut is_first = true;

        for x in self.1 {
            if is_first {
                is_first = false;
            } else {
                write!(f, "{}", self.0)?;
            }
            write!(f, "{x}")?;
        }

        Ok(())
    }
}

pub struct TerminateEach<'a, T>(pub &'a str, pub &'a Vec<T>)
where
    T: Display;

impl<'a, T> Display for TerminateEach<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        self.1
            .iter()
            .try_fold((), |_, x| write!(f, "{}{}", x, self.0))
    }
}

pub struct Indented<T>(pub T)
where
    T: Display;

impl<T> Display for Indented<T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(indented(f), "{}", self.0)
    }
}

pub struct Block<T>(pub T)
where
    T: Display;

impl<T> Display for Block<T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        write!(BlockFormatter::new(f), "{}", self.0)
    }
}

struct BlockFormatter<'a, T>
where
    T: Write,
{
    inner: &'a mut T,
    has_written: bool,
}

impl<'a, T> BlockFormatter<'a, T>
where
    T: Write,
{
    pub fn new(inner: &'a mut T) -> Self {
        Self {
            inner,
            has_written: false,
        }
    }
}

impl<'a, T> Write for BlockFormatter<'a, T>
where
    T: Write,
{
    fn write_str(&mut self, s: &str) -> std::fmt::Result {
        match (self.has_written, s) {
            (false, _) if !s.is_empty() => {
                self.has_written = true;
                write!(self.inner, "\n{s}")
            }
            _ => self.inner.write_str(s),
        }
    }
}