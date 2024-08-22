mod component;
mod declaration;
mod expression;
mod module;
mod parameter;
mod statement;
mod types;

use kore::format::{indented, SeparateEach, SuffixEach};
use std::fmt::{Display, Formatter, Result, Write};

struct Typedef<'a, TypeExpression>(&'a Option<TypeExpression>);

impl<'a, TypeExpression> Display for Typedef<'a, TypeExpression>
where
    TypeExpression: Display,
{
    fn fmt(&self, f: &mut Formatter) -> Result {
        if let Some(typedef) = self.0 {
            write!(f, ": {}", typedef)
        } else {
            Ok(())
        }
    }
}

struct Parameters<'a, T>(&'a Vec<T>)
where
    T: Display;

impl<'a, T> Display for Parameters<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> Result {
        if self.0.is_empty() {
            Ok(())
        } else {
            write!(f, "({})", SeparateEach(", ", self.0))
        }
    }
}

pub struct Lambda<'a, P, R>(pub &'a [P], pub R);

impl<'a, P, R> Display for Lambda<'a, P, R>
where
    P: Display,
    R: Display,
{
    fn fmt(&self, f: &mut Formatter) -> Result {
        write!(
            f,
            "({parameters}) -> {result}",
            parameters = SeparateEach(", ", self.0),
            result = self.1
        )
    }
}

pub struct Object<T, I>(pub I)
where
    I: IntoIterator<Item = T>;

impl<T, I> Display for Object<T, I>
where
    T: Clone + Display,
    I: Clone + IntoIterator<Item = T>,
{
    fn fmt(&self, f: &mut Formatter) -> Result {
        let items = self.0.clone().into_iter().collect::<Vec<_>>();

        if items.is_empty() {
            write!(f, "{{}}")
        } else {
            writeln!(f, "{{")?;
            write!(indented(f), "{}", SuffixEach(",\n", items))?;
            write!(f, "}}")
        }
    }
}
