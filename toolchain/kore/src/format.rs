use std::fmt::{Display, Formatter, Write};

pub fn indented<T>(f: &mut T) -> indenter::Indented<T>
where
    T: Sized,
{
    indenter::indented(f).with_str("  ")
}

pub struct SeparateEach<T, U, I>(pub T, pub I)
where
    T: Display,
    U: Display,
    I: IntoIterator<Item = U>;

impl<T, U, I> Display for SeparateEach<T, U, I>
where
    T: Display,
    U: Display,
    I: Clone + IntoIterator<Item = U>,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        let mut is_first = true;

        for x in self.1.clone() {
            if is_first {
                is_first = false;
            } else {
                self.0.fmt(f)?;
            }
            x.fmt(f)?;
        }

        Ok(())
    }
}

pub struct PrefixEach<'a, T, I>(pub &'a str, pub I)
where
    T: Display,
    I: IntoIterator<Item = T>;

impl<T, I> Display for PrefixEach<'_, T, I>
where
    T: Display,
    I: Clone + IntoIterator<Item = T>,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        for x in self.1.clone() {
            write!(f, "{}{}", self.0, x)?;
        }

        Ok(())
    }
}

pub struct SuffixEach<'a, T, I>(pub &'a str, pub I)
where
    T: Display,
    I: IntoIterator<Item = T>;

impl<T, I> Display for SuffixEach<'_, T, I>
where
    T: Display,
    I: Clone + IntoIterator<Item = T>,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        for x in self.1.clone() {
            write!(f, "{}{}", x, self.0)?;
        }

        Ok(())
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

impl<T> Write for BlockFormatter<'_, T>
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
