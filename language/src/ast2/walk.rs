use super::{raw, Range};

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub struct NodeId(pub usize);

pub trait Visit {
    type Context;
    type Expr;
    type Stmt;
    type Comp;
    type TExpr;
    type Param;
    type Decl;
    type Imp;
    type Mod;

    fn expression(
        &self,
        x: super::Expression<Self::Expr, Self::Stmt, Self::Comp>,
        r: Range,
        c: Self::Context,
    ) -> (Self::Expr, Self::Context);

    fn statement(
        &self,
        x: super::Statement<Self::Expr>,
        r: Range,
        c: Self::Context,
    ) -> (Self::Stmt, Self::Context);

    fn component(
        &self,
        x: super::Component<Self::Expr, Self::Comp>,
        r: Range,
        c: Self::Context,
    ) -> (Self::Comp, Self::Context);

    fn type_expression(
        &self,
        x: super::TypeExpression<Self::TExpr>,
        r: Range,
        c: Self::Context,
    ) -> (Self::TExpr, Self::Context);

    fn parameter(
        &self,
        x: super::Parameter<Self::Expr, Self::TExpr>,
        r: Range,
        c: Self::Context,
    ) -> (Self::Param, Self::Context);

    fn declaration(
        &self,
        x: super::Declaration<Self::Expr, Self::Param, Self::Mod, Self::TExpr>,
        r: Range,
        c: Self::Context,
    ) -> (Self::Decl, Self::Context);

    fn import(&self, x: super::Import, r: Range, c: Self::Context) -> (Self::Imp, Self::Context);

    fn module(
        &self,
        x: super::Module<Self::Imp, Self::Decl>,
        r: Range,
        c: Self::Context,
    ) -> (Self::Mod, Self::Context);
}

pub trait Walk<Context, Visitor>
where
    Visitor: Visit<Context = Context>,
{
    type Output;

    fn walk(self, visitor: &Visitor, context: Context) -> (Self::Output, Context);
}

impl<Key, Target, Context, Visitor> Walk<Context, Visitor> for (Key, Target)
where
    Target: Walk<Context, Visitor>,
    Visitor: Visit<Context = Context>,
{
    type Output = (Key, Target::Output);

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context) {
        let (key, x) = self;
        let (x, c) = x.walk(v, c);

        ((key, x), c)
    }
}

impl<Target, Context, Visitor> Walk<Context, Visitor> for Option<Target>
where
    Target: Walk<Context, Visitor>,
    Visitor: Visit<Context = Context>,
{
    type Output = Option<Target::Output>;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context) {
        if let Some(x) = self {
            let (x, c) = x.walk(v, c);
            (Some(x), c)
        } else {
            (None, c)
        }
    }
}

impl<Target, Context, Visitor> Walk<Context, Visitor> for Vec<Target>
where
    Target: Walk<Context, Visitor>,
    Visitor: Visit<Context = Context>,
{
    type Output = Vec<Target::Output>;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context) {
        self.into_iter().fold((vec![], c), |(mut acc, c), x| {
            let (x, c) = x.walk(v, c);
            acc.push(x);
            (acc, c)
        })
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::Expression
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Expr;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context)
    where
        Visitor: Visit<Context = Context>,
    {
        let raw::Node { value, range } = self.0;

        match value {
            super::Expression::Primitive(x) => {
                v.expression(super::Expression::Primitive(x), range, c)
            }

            super::Expression::Identifier(x) => {
                v.expression(super::Expression::Identifier(x), range, c)
            }

            super::Expression::Group(x) => {
                let (x, c) = x.walk(v, c);

                v.expression(super::Expression::Group(Box::new(x)), range, c)
            }

            super::Expression::Closure(xs) => {
                let (xs, c) = xs.walk(v, c);

                v.expression(super::Expression::Closure(xs), range, c)
            }

            super::Expression::UnaryOperation(op, x) => {
                let (x, c) = x.walk(v, c);

                v.expression(super::Expression::UnaryOperation(op, Box::new(x)), range, c)
            }

            super::Expression::BinaryOperation(op, l, r) => {
                let (l, c) = l.walk(v, c);
                let (r, c) = r.walk(v, c);

                v.expression(
                    super::Expression::BinaryOperation(op, Box::new(l), Box::new(r)),
                    range,
                    c,
                )
            }

            super::Expression::PropertyAccess(x, property) => {
                let (x, c) = x.walk(v, c);

                v.expression(
                    super::Expression::PropertyAccess(Box::new(x), property),
                    range,
                    c,
                )
            }

            super::Expression::FunctionCall(x, arguments) => {
                let (x, c) = x.walk(v, c);
                let (arguments, c) = arguments.walk(v, c);

                v.expression(
                    super::Expression::FunctionCall(Box::new(x), arguments),
                    range,
                    c,
                )
            }

            super::Expression::Component(x) => {
                let (x, c) = x.walk(v, c);

                v.expression(super::Expression::Component(Box::new(x)), range, c)
            }

            super::Expression::Style(xs) => {
                let (xs, c) = xs.walk(v, c);

                v.expression(super::Expression::Style(xs), range, c)
            }
        }
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::Statement
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Stmt;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context)
    where
        Visitor: Visit<Context = Context>,
    {
        let raw::Node { value, range } = self.0;

        match value {
            super::Statement::Expression(x) => {
                let (x, c) = x.walk(v, c);

                v.statement(super::Statement::Expression(x), range, c)
            }

            super::Statement::Variable(binding, x) => {
                let (x, c) = x.walk(v, c);

                v.statement(super::Statement::Variable(binding, x), range, c)
            }
        }
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::Component
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Comp;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context)
    where
        Visitor: Visit<Context = Context>,
    {
        let raw::Node { value, range } = self.0;

        match value {
            super::Component::Text(x) => v.component(super::Component::Text(x), range, c),

            super::Component::Expression(x) => {
                let (x, c) = x.walk(v, c);

                v.component(super::Component::Expression(x), range, c)
            }

            super::Component::Fragment(xs) => {
                let (xs, c) = xs.walk(v, c);

                v.component(super::Component::Fragment(xs), range, c)
            }

            super::Component::ClosedElement(tag, attributes) => {
                let (attributes, c) = attributes.walk(v, c);

                v.component(super::Component::ClosedElement(tag, attributes), range, c)
            }

            super::Component::OpenElement(start_tag, attributes, children, end_tag) => {
                let (attributes, c) = attributes.walk(v, c);
                let (children, c) = children.walk(v, c);

                v.component(
                    super::Component::OpenElement(start_tag, attributes, children, end_tag),
                    range,
                    c,
                )
            }
        }
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::TypeExpression
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::TExpr;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context)
    where
        Visitor: Visit<Context = Context>,
    {
        let raw::Node { value, range } = self.0;

        match value {
            super::TypeExpression::Primitive(x) => {
                v.type_expression(super::TypeExpression::Primitive(x), range, c)
            }

            super::TypeExpression::Identifier(x) => {
                v.type_expression(super::TypeExpression::Identifier(x), range, c)
            }

            super::TypeExpression::Group(x) => {
                let (x, c) = x.walk(v, c);

                v.type_expression(super::TypeExpression::Group(Box::new(x)), range, c)
            }

            super::TypeExpression::PropertyAccess(x, property) => {
                let (x, c) = x.walk(v, c);

                v.type_expression(
                    super::TypeExpression::PropertyAccess(Box::new(x), property),
                    range,
                    c,
                )
            }

            super::TypeExpression::Function(parameters, x) => {
                let (parameters, c) = parameters.walk(v, c);
                let (x, c) = x.walk(v, c);

                v.type_expression(
                    super::TypeExpression::Function(parameters, Box::new(x)),
                    range,
                    c,
                )
            }
        }
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::Parameter
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Param;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context) {
        let raw::Node {
            value:
                super::Parameter {
                    name,
                    value_type,
                    default_value,
                },
            range,
        } = self.0;
        let (value_type, c) = value_type.walk(v, c);
        let (default_value, c) = default_value.walk(v, c);

        v.parameter(
            super::Parameter {
                name,
                value_type,
                default_value,
            },
            range,
            c,
        )
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::Declaration
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Decl;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context) {
        let raw::Node { value, range } = self.0;

        match value {
            super::Declaration::TypeAlias { storage, value } => {
                let (value, c) = value.walk(v, c);

                v.declaration(super::Declaration::TypeAlias { storage, value }, range, c)
            }

            super::Declaration::Constant {
                storage,
                value_type,
                value,
            } => {
                let (value_type, c) = value_type.walk(v, c);
                let (value, c) = value.walk(v, c);

                v.declaration(
                    super::Declaration::Constant {
                        storage,
                        value_type,
                        value,
                    },
                    range,
                    c,
                )
            }

            super::Declaration::Enumerated { storage, variants } => {
                let (variants, c) = variants.walk(v, c);

                v.declaration(
                    super::Declaration::Enumerated { storage, variants },
                    range,
                    c,
                )
            }

            super::Declaration::Function {
                storage,
                parameters,
                body_type,
                body,
            } => {
                let (parameters, c) = parameters.walk(v, c);
                let (body_type, c) = body_type.walk(v, c);
                let (body, c) = body.walk(v, c);

                v.declaration(
                    super::Declaration::Function {
                        storage,
                        parameters,
                        body_type,
                        body,
                    },
                    range,
                    c,
                )
            }

            super::Declaration::View {
                storage,
                parameters,
                body,
            } => {
                let (parameters, c) = parameters.walk(v, c);
                let (body, c) = body.walk(v, c);

                v.declaration(
                    super::Declaration::View {
                        storage,
                        parameters,
                        body,
                    },
                    range,
                    c,
                )
            }

            super::Declaration::Module { storage, value } => {
                let (value, c) = value.walk(v, c);

                v.declaration(super::Declaration::Module { storage, value }, range, c)
            }
        }
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::Import
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Imp;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context) {
        let raw::Node {
            value:
                super::Import {
                    source,
                    path,
                    alias,
                },
            range,
        } = self.0;

        v.import(
            super::Import {
                source,
                path,
                alias,
            },
            range,
            c,
        )
    }
}

impl<Context, Visitor> Walk<Context, Visitor> for raw::Module
where
    Visitor: Visit<Context = Context>,
{
    type Output = Visitor::Mod;

    fn walk(self, v: &Visitor, c: Context) -> (Self::Output, Context) {
        let raw::Node {
            value:
                super::Module {
                    imports,
                    declarations,
                },
            range,
        } = self.0;
        let (imports, c) = imports.walk(v, c);
        let (declarations, c) = declarations.walk(v, c);

        v.module(
            super::Module {
                imports,
                declarations,
            },
            range,
            c,
        )
    }
}
