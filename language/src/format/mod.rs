mod declaration;
mod expression;
mod ksx;
mod module;
mod parameter;
mod statement;
mod type_expression;

use crate::ast::TypeExpressionNode;
use kore::format::SeparateEach;
use std::fmt::{Display, Formatter};

struct Typedef<'a, R, C>(&'a Option<TypeExpressionNode<R, C>>);

impl<'a, R, C> Display for Typedef<'a, R, C>
where
    R: Copy,
{
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
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
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        if self.0.is_empty() {
            Ok(())
        } else {
            write!(f, "({})", SeparateEach(", ", self.0))
        }
    }
}
