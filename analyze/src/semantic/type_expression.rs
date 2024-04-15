use super::Visitor;
use crate::error::Error;
use lang::{ast, walk::CommonVisitor};

pub fn analyze(
    x: &ast::TypeExpression<
        <Visitor as CommonVisitor>::Binding,
        <Visitor as CommonVisitor>::TypeExpression,
    >,
    _: &<Visitor as CommonVisitor>::Context,
    _: &Visitor,
) -> Option<Vec<Error>> {
    match x {
        ast::TypeExpression::Primitive(_) => None,

        ast::TypeExpression::Identifier(_) => None,

        ast::TypeExpression::Group(_) => None,

        ast::TypeExpression::PropertyAccess(..) => None,

        ast::TypeExpression::Function(..) => None,

        ast::TypeExpression::Object(entries) => {
            let mut names = vec![];
            let mut errors = vec![];

            for entry in entries {
                if let Some(name) = entry.binding().map(ast::meta::Binding::name) {
                    if names.contains(&name) {
                        errors.push(Error::DuplicateProperty(name.to_owned()));
                    } else {
                        names.push(name);
                    }
                }
            }

            (!errors.is_empty()).then_some(errors)
        }
    }
}
