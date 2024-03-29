mod component;
mod declaration;
mod expression;
mod module;
mod parameter;
mod statement;
mod type_expression;

use kore::format::SeparateEach;
use std::fmt::{Display, Formatter};

struct Typedef<'a, TypeExpression>(&'a Option<TypeExpression>);

impl<'a, TypeExpression> Display for Typedef<'a, TypeExpression>
where
    TypeExpression: Display,
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
