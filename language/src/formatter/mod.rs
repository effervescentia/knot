mod declaration;
mod expression;
mod ksx;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::{ast::type_expression::TypeExpressionNode, common::position::Decrement};
use combine::Stream;
use std::fmt::{Debug, Display, Formatter};

struct Typedef<'a, T, C>(&'a Option<TypeExpressionNode<T, C>>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<'a, T, C> Display for Typedef<'a, T, C>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if let Some(typedef) = self.0 {
            write!(f, ": {}", typedef)
        } else {
            Ok(())
        }
    }
}

struct SeparatedBy<'a, T>(&'a str, &'a Vec<T>)
where
    T: Display;

impl<'a, T> Display for SeparatedBy<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if self.1.is_empty() {
            Ok(())
        } else {
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
            write!(f, "({})", SeparatedBy(", ", self.0))
        }
    }
}

struct Statements<'a, T>(&'a Vec<T>)
where
    T: Display;

impl<'a, T> Display for Statements<'a, T>
where
    T: Display,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if self.0.is_empty() {
            Ok(())
        } else {
            self.0.iter().fold(write!(f, "\n"), |acc, x| {
                acc.and_then(|_| write!(f, "{x};\n"))
            })
        }
    }
}
