use crate::{
    javascript::{Expression, Statement},
    Options,
};
use knot_language::ast::{self, storage::Storage, DeclarationShape, ModuleShape, StatementShape};

fn parameter_name(suffix: &String) -> String {
    format!("$param_{suffix}")
}

fn minified_parameter(x: usize) -> String {
    const FIRST_CHAR: u32 = 97;

    match x {
        _ if x < 0 => String::from("a"),

        _ => {
            let c = char::from_u32((x as u32 % 25) + FIRST_CHAR).unwrap();

            std::iter::repeat(c).take(x / 25).collect::<String>()
        }
    }
}

impl Statement {
    pub fn from_statement(value: &StatementShape, opts: &Options) -> Vec<Self> {
        match &value.0 {
            ast::Statement::Expression(x) => {
                vec![Self::Expression(Expression::from_expression(x, opts))]
            }

            ast::Statement::Variable(name, x) => {
                vec![Self::Variable(
                    name.clone(),
                    Expression::from_expression(x, opts),
                )]
            }
        }
    }

    pub fn from_last_statement(value: &StatementShape, opts: &Options) -> Vec<Self> {
        match &value.0 {
            ast::Statement::Expression(x) => {
                vec![Self::Return(Some(Expression::from_expression(x, opts)))]
            }

            ast::Statement::Variable(name, x) => {
                vec![
                    Self::Variable(name.clone(), Expression::from_expression(x, opts)),
                    Self::Return(None),
                ]
            }
        }
    }

    pub fn from_declaration(value: &DeclarationShape, opts: &Options) -> Vec<Self> {
        fn parameter_name(suffix: &String) -> String {
            format!("$param_{suffix}")
        }

        match &value.0 {
            ast::Declaration::TypeAlias { .. } => vec![],

            ast::Declaration::Enumerated {
                name: Storage(_, name),
                variants,
            } => vec![Self::Variable(
                name.clone(),
                Expression::Object(
                    variants
                        .iter()
                        .map(|(variant_name, variant_parameters)| {
                            let parameters = variant_parameters
                                .iter()
                                .enumerate()
                                .map(|(index, _)| parameter_name(&index.to_string()))
                                .collect::<Vec<_>>();

                            let results = vec![
                                vec![Expression::DotAccess(
                                    Box::new(Expression::Identifier(name.clone())),
                                    variant_name.clone(),
                                )],
                                parameters
                                    .iter()
                                    .map(|x| Expression::Identifier(x.clone()))
                                    .collect(),
                            ]
                            .concat();

                            (
                                variant_name.clone(),
                                Expression::Function(
                                    Some(variant_name.clone()),
                                    parameters,
                                    vec![Self::Return(Some(Expression::Array(results)))],
                                ),
                            )
                        })
                        .collect(),
                ),
            )],

            ast::Declaration::Constant {
                name: Storage(_, name),
                value,
                ..
            } => vec![Self::Variable(
                name.clone(),
                Expression::from_expression(value, opts),
            )],

            ast::Declaration::Function {
                name: Storage(_, name),
                parameters,
                body,
                ..
            }
            | ast::Declaration::View {
                name: Storage(_, name),
                parameters,
                body,
                ..
            } => {
                let statements = match Expression::from_expression(body, opts) {
                    Expression::Closure(xs) => xs,

                    x => vec![Self::Return(Some(x))],
                };

                vec![Self::Expression(Expression::Function(
                    Some(name.clone()),
                    parameters.iter().map(|x| x.0.name.clone()).collect(),
                    statements,
                ))]
            }

            ast::Declaration::Module {
                name: Storage(_, name),
                value,
            } => {
                let mut statements = Self::from_module(value, opts);
                statements.push(Self::Return(Some(Expression::Object(
                    value
                        .0
                        .declarations
                        .iter()
                        .filter_map(|x| {
                            if x.0.is_public() {
                                Some((
                                    x.0.name().clone(),
                                    Expression::Identifier(x.0.name().clone()),
                                ))
                            } else {
                                None
                            }
                        })
                        .collect(),
                ))));

                vec![Self::Variable(
                    name.clone(),
                    Expression::Closure(statements),
                )]
            }
        }
    }

    pub fn from_module(value: &ModuleShape, opts: &Options) -> Vec<Self> {
        value
            .0
            .declarations
            .iter()
            .flat_map(|x| Self::from_declaration(x, opts))
            .collect()
    }
}
