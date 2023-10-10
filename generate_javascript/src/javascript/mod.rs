mod expression;
mod statement;

use crate::Options;
use knot_language::ast::ModuleShape;
use statement::Statement;

pub struct JavaScript(Vec<Statement>);

impl JavaScript {
    fn from_module(value: &ModuleShape, opts: &Options) -> Self {
        let statements = vec![
            Statement::from_module(value, opts),
            value
                .0
                .declarations
                .iter()
                .filter_map(|x| {
                    if x.0.is_public() {
                        Some(Statement::Export(x.0.name().clone()))
                    } else {
                        None
                    }
                })
                .collect(),
        ]
        .concat();

        Self(Statement::from_module(value, opts))
    }
}
