use super::Transform;
use crate::parser::{
    declaration::{parameter::Parameter, Declaration},
    expression::{ksx::KSX, statement::Statement, Expression},
    module::Module,
};

impl<'a, E1, K1, E2, K2, FE, FK> Transform<(&'a FE, &'a FK), Expression<E2, K2>>
    for Expression<E1, K1>
where
    FE: Fn(E1) -> E2,
    FK: Fn(K1) -> K2,
{
    fn transform(self, ctx: (&'a FE, &'a FK)) -> (Expression<E2, K2>, (&'a FE, &'a FK)) {
        match self {
            Expression::Primitive(x) => (Expression::Primitive(x), ctx),
            Expression::Identifier(x) => (Expression::Identifier(x), ctx),
            Expression::Group(x) => (Expression::Group(Box::new(ctx.0(*x))), ctx),
            Expression::Closure(xs) => (
                Expression::Closure(
                    xs.into_iter()
                        .map(|x| x.transform(ctx.0).0)
                        .collect::<Vec<_>>(),
                ),
                ctx,
            ),
            Expression::UnaryOperation(op, x) => {
                (Expression::UnaryOperation(op, Box::new(ctx.0(*x))), ctx)
            }
            Expression::BinaryOperation(op, lhs, rhs) => (
                Expression::BinaryOperation(op, Box::new(ctx.0(*lhs)), Box::new(ctx.0(*rhs))),
                ctx,
            ),
            Expression::DotAccess(lhs, rhs) => {
                (Expression::DotAccess(Box::new(ctx.0(*lhs)), rhs), ctx)
            }
            Expression::FunctionCall(x, args) => (
                Expression::FunctionCall(
                    Box::new(ctx.0(*x)),
                    args.into_iter().map(|x| ctx.0(x)).collect::<Vec<_>>(),
                ),
                ctx,
            ),
            Expression::Style(xs) => (
                Expression::Style(
                    xs.into_iter()
                        .map(|(name, x)| (name, ctx.0(x)))
                        .collect::<Vec<_>>(),
                ),
                ctx,
            ),
            Expression::KSX(x) => (Expression::KSX(Box::new(ctx.1(*x))), ctx),
        }
    }
}

impl<E1, E2, F> Transform<&F, Statement<E2>> for Statement<E1>
where
    F: Fn(E1) -> E2,
{
    fn transform(self, ctx: &F) -> (Statement<E2>, &F) {
        match self {
            Statement::Effect(x) => (Statement::Effect(ctx(x)), ctx),
            Statement::Variable(name, x) => (Statement::Variable(name, ctx(x)), ctx),
        }
    }
}

impl<'a, E1, K1, E2, K2, FE, FK> Transform<(&'a FE, &'a FK), KSX<E2, K2>> for KSX<E1, K1>
where
    FE: Fn(E1) -> E2,
    FK: Fn(K1) -> K2,
{
    fn transform(self, ctx: (&'a FE, &'a FK)) -> (KSX<E2, K2>, (&'a FE, &'a FK)) {
        let transform_attributes = |xs: Vec<(String, Option<E1>)>| {
            xs.into_iter()
                .map(|(name, value)| (name, value.map(ctx.0)))
                .collect::<Vec<_>>()
        };

        match self {
            KSX::Text(x) => (KSX::Text(x), ctx),
            KSX::Inline(x) => (KSX::Inline(ctx.0(x)), ctx),
            KSX::Fragment(children) => (
                KSX::Fragment(children.into_iter().map(|x| ctx.1(x)).collect::<Vec<_>>()),
                ctx,
            ),
            KSX::ClosedElement(tag, attributes) => (
                KSX::ClosedElement(tag, transform_attributes(attributes)),
                ctx,
            ),
            KSX::OpenElement(start_tag, attributes, children, end_tag) => (
                KSX::OpenElement(
                    start_tag,
                    transform_attributes(attributes),
                    children.into_iter().map(|x| ctx.1(x)).collect::<Vec<_>>(),
                    end_tag,
                ),
                ctx,
            ),
        }
    }
}

impl<'a, E1, M1, T1, E2, M2, T2, FE, FM, FT>
    Transform<(&'a FE, &'a FM, &'a FT), Declaration<E2, M2, T2>> for Declaration<E1, M1, T1>
where
    FE: Fn(E1) -> E2,
    FM: Fn(M1) -> M2,
    FT: Fn(T1) -> T2,
{
    fn transform(
        self,
        ctx: (&'a FE, &'a FM, &'a FT),
    ) -> (Declaration<E2, M2, T2>, (&'a FE, &'a FM, &'a FT)) {
        let transform_parameters = |xs: Vec<Parameter<E1, T1>>| {
            xs.into_iter()
                .map(
                    |Parameter {
                         name,
                         value_type,
                         default_value,
                     }| {
                        Parameter {
                            name,
                            value_type: value_type.map(ctx.2),
                            default_value: default_value.map(ctx.0),
                        }
                    },
                )
                .collect::<Vec<_>>()
        };

        match self {
            Declaration::TypeAlias { name, value } => (
                Declaration::TypeAlias {
                    name,
                    value: ctx.2(value),
                },
                ctx,
            ),
            Declaration::Constant {
                name,
                value_type,
                value,
            } => (
                Declaration::Constant {
                    name,
                    value_type: value_type.map(ctx.2),
                    value: ctx.0(value),
                },
                ctx,
            ),
            Declaration::Enumerated { name, variants } => (
                Declaration::Enumerated {
                    name,
                    variants: variants
                        .into_iter()
                        .map(|(key, value)| (key, value.into_iter().map(ctx.2).collect::<Vec<_>>()))
                        .collect::<Vec<_>>(),
                },
                ctx,
            ),
            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => (
                Declaration::Function {
                    name,
                    parameters: transform_parameters(parameters),
                    body_type: body_type.map(ctx.2),
                    body: ctx.0(body),
                },
                ctx,
            ),
            Declaration::View {
                name,
                parameters,
                body,
            } => (
                Declaration::View {
                    name,
                    parameters: transform_parameters(parameters),
                    body: ctx.0(body),
                },
                ctx,
            ),
            Declaration::Module { name, value } => (
                Declaration::Module {
                    name,
                    value: ctx.1(value),
                },
                ctx,
            ),
        }
    }
}

impl<'a, D1, D2, F> Transform<&'a F, Module<D2>> for Module<D1>
where
    F: Fn(D1) -> D2,
{
    fn transform(self, ctx: &'a F) -> (Module<D2>, &'a F) {
        (
            Module {
                imports: self.imports,
                declarations: self
                    .declarations
                    .into_iter()
                    .map(|x| ctx(x))
                    .collect::<Vec<_>>(),
            },
            ctx,
        )
    }
}
