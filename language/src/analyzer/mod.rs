// pub mod transform;
pub mod weak;
use crate::parser::{
    declaration::{parameter::Parameter, storage::Storage, Declaration, DeclarationNode},
    expression::{
        binary_operation::BinaryOperator,
        ksx::{KSXNode, KSX},
        primitive::Primitive,
        statement::Statement,
        Expression, ExpressionNode, UnaryOperator,
    },
    module::{
        import::{Import, Target},
        Module, ModuleNode,
    },
    node::Node,
    position::Decrement,
    range::Ranged,
    types::type_expression::{TypeExpression, TypeExpressionNode},
};
use combine::Stream;
use std::fmt::Debug;
use std::{
    cell::RefCell,
    collections::{BTreeMap, HashMap},
};
// use weak::ExpressionWeak;
// use self::weak::WeakContext;

pub trait Transform<C, O> {
    fn transform(self, ctx: C) -> (O, C);
}

// pub trait Identify<'a, I, O>: Transform<WeakContext<'a>, I, O> {
//     fn identify(x: I, ctx: WeakContext<'a>) -> (O, WeakContext<'a>) {
//         Self::transform(x, ctx)
//     }
// }

// enum AST<'a> {
enum AST {
    // Expression(&'a Expression),
    // Statement(&'a Statement<&'a Expression>),
    // Declaration(&'a Declaration<_, _>),
}

#[derive(Debug, Clone, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
    Enumerated(Vec<(String, Vec<T>)>),
    Function(Vec<T>, T),
    View(Vec<T>, T),
    Module(Vec<(String, T)>),
}

struct Context {
    next_id: i32,
}

impl Context {
    pub fn new() -> Self {
        Self { next_id: 0 }
    }

    pub fn generate_id(&mut self) -> i32 {
        let id = self.next_id;
        self.next_id += 1;
        id
    }
}

fn analyze<T>(x: ModuleNode<T, ()>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let mut ctx = Context::new();

    let x = analyze_module(x, &mut ctx);
}

fn analyze_module<T>(x: ModuleNode<T, ()>, ctx: &mut Context) -> ModuleNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let declarations =
        x.0.declarations
            .into_iter()
            .map(|x| analyze_declaration(x, ctx))
            .collect::<Vec<_>>();

    ModuleNode(
        Module {
            imports: x.0.imports,
            declarations,
        },
        ctx.generate_id(),
    )
}

fn analyze_declaration<T>(x: DeclarationNode<T, ()>, ctx: &mut Context) -> DeclarationNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let analyze_parameters =
        |xs: Vec<Parameter<ExpressionNode<T, ()>, TypeExpressionNode<T, ()>>>,
         ctx: &mut Context| {
            xs.into_iter()
                .map(
                    |Parameter {
                         name,
                         value_type,
                         default_value,
                     }| Parameter {
                        name,
                        value_type: value_type.map(|x| analyze_type_expression(x, ctx)),
                        default_value: default_value.map(|x| analyze_expression(x, ctx)),
                    },
                )
                .collect::<Vec<_>>()
        };

    DeclarationNode(
        x.0.map(|x| match x {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name,
                value: analyze_type_expression(value, ctx),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name,
                variants: variants
                    .into_iter()
                    .map(|(name, xs)| {
                        (
                            name,
                            xs.into_iter()
                                .map(|x| analyze_type_expression(x, ctx))
                                .collect::<Vec<_>>(),
                        )
                    })
                    .collect::<Vec<_>>(),
            },

            Declaration::Constant {
                name,
                value_type,
                value,
            } => Declaration::Constant {
                name,
                value_type: value_type.map(|x| analyze_type_expression(x, ctx)),
                value: analyze_expression(value, ctx),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name,
                parameters: analyze_parameters(parameters, ctx),
                body_type: body_type.map(|x| analyze_type_expression(x, ctx)),
                body: analyze_expression(body, ctx),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name,
                parameters: analyze_parameters(parameters, ctx),
                body: analyze_expression(body, ctx),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name,
                value: analyze_module(value, ctx),
            },
        })
        .with_context(ctx.generate_id()),
    )
}

fn analyze_expression<T>(x: ExpressionNode<T, ()>, ctx: &mut Context) -> ExpressionNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    ExpressionNode(
        x.0.map(|x| match x {
            Expression::Primitive(x) => Expression::Primitive(x),

            Expression::Identifier(x) => Expression::Identifier(x),

            Expression::Group(x) => Expression::Group(Box::new(analyze_expression(*x, ctx))),

            Expression::Closure(xs) => Expression::Closure(
                xs.into_iter()
                    .map(|x| match x {
                        Statement::Effect(x) => Statement::Effect(analyze_expression(x, ctx)),
                        Statement::Variable(name, x) => {
                            Statement::Variable(name, analyze_expression(x, ctx))
                        }
                    })
                    .collect::<Vec<_>>(),
            ),

            Expression::UnaryOperation(op, lhs) => {
                Expression::UnaryOperation(op, Box::new(analyze_expression(*lhs, ctx)))
            }

            Expression::BinaryOperation(op, lhs, rhs) => Expression::BinaryOperation(
                op,
                Box::new(analyze_expression(*lhs, ctx)),
                Box::new(analyze_expression(*rhs, ctx)),
            ),

            Expression::DotAccess(lhs, rhs) => {
                Expression::DotAccess(Box::new(analyze_expression(*lhs, ctx)), rhs)
            }

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new(analyze_expression(*x, ctx)),
                args.into_iter()
                    .map(|x| analyze_expression(x, ctx))
                    .collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key, analyze_expression(value, ctx)))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new(analyze_ksx(*x, ctx))),
        })
        .with_context(ctx.generate_id()),
    )
}

fn analyze_ksx<T>(x: KSXNode<T, ()>, ctx: &mut Context) -> KSXNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let analyze_attributes = |xs: Vec<(String, Option<ExpressionNode<T, ()>>)>,
                              ctx: &mut Context| {
        xs.into_iter()
            .map(|(key, value)| (key, value.map(|x| analyze_expression(x, ctx))))
            .collect::<Vec<_>>()
    };

    let analyze_children = |xs: Vec<KSXNode<T, ()>>, ctx: &mut Context| {
        xs.into_iter()
            .map(|x| analyze_ksx(x, ctx))
            .collect::<Vec<_>>()
    };

    KSXNode(
        x.0.map(|x| match x {
            KSX::Text(x) => KSX::Text(x),

            KSX::Inline(x) => KSX::Inline(analyze_expression(x, ctx)),

            KSX::Fragment(children) => KSX::Fragment(analyze_children(children, ctx)),

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag, analyze_attributes(attributes, ctx))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag,
                analyze_attributes(attributes, ctx),
                analyze_children(children, ctx),
                end_tag,
            ),
        })
        .with_context(ctx.generate_id()),
    )
}

fn analyze_type_expression<T>(
    x: TypeExpressionNode<T, ()>,
    ctx: &mut Context,
) -> TypeExpressionNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    TypeExpressionNode(
        x.0.map(|x| match x {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x),

            TypeExpression::Group(x) => {
                TypeExpression::Group(Box::new(analyze_type_expression(*x, ctx)))
            }

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new(analyze_type_expression(*lhs, ctx)), rhs)
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params
                    .into_iter()
                    .map(|x| analyze_type_expression(x, ctx))
                    .collect::<Vec<_>>(),
                Box::new(analyze_type_expression(*body, ctx)),
            ),
        })
        .with_context(ctx.generate_id()),
    )
}

// fn primitive(x: Primitive, ctx: Context) -> (usize, Context) {
//     let typ = match x {
//         Primitive::Nil => Type::Nil,
//         Primitive::Boolean(_) => Type::Boolean,
//         Primitive::Integer(_) => Type::Integer,
//         Primitive::Float(_, _) => Type::Float,
//         Primitive::String(_) => Type::String,
//     };

//     ctx.add_strong(typ)
// }

// fn identifier(name: String, ctx: Context) -> (usize, Context) {
//     ctx.resolve(&name)
// }

// fn group(x: Expression, ctx: Context) -> (usize, Context) {
//     Context::refer(expression(x, ctx))
// }

// fn closure(xs: Vec<Statement<Expression>>, ctx: Context) -> (usize, Context) {
//     let child_ctx = ctx.child();
//     let (id, _) = xs.into_iter().fold((None, child_ctx), |(_, ctx), x| {
//         let (id, ctx) = statement(x, ctx);
//         (Some(id), ctx)
//     });

//     match id {
//         Some(id) => Context::refer((id, ctx)),
//         None => ctx.add_strong(Type::Nil),
//     }
// }

// fn unary_operation(op: UnaryOperator, x: Expression, ctx: Context) -> (usize, Context) {
//     Context::refer(match op {
//         UnaryOperator::Not => {
//             let (_, ctx) = expression(x, ctx);
//             ctx.add_strong(Type::Boolean)
//         }
//         UnaryOperator::Absolute | UnaryOperator::Negate => expression(x, ctx),
//     })
// }

// fn binary_operation(
//     op: BinaryOperator,
//     lhs: Expression,
//     rhs: Expression,
//     ctx: Context,
// ) -> (usize, Context) {
//     let (_, ctx) = expression(lhs, ctx);
//     let (_, ctx) = expression(rhs, ctx);

//     match op {
//         BinaryOperator::And
//         | BinaryOperator::Or
//         | BinaryOperator::GreaterThan
//         | BinaryOperator::GreaterThanOrEqual
//         | BinaryOperator::LessThan
//         | BinaryOperator::LessThanOrEqual
//         | BinaryOperator::Equal
//         | BinaryOperator::NotEqual => ctx.add_strong(Type::Boolean),

//         BinaryOperator::Add | BinaryOperator::Subtract | BinaryOperator::Multiply => {
//             ctx.add_type(WeakType::Number)
//         }

//         BinaryOperator::Divide | BinaryOperator::Exponent => ctx.add_strong(Type::Float),
//     }
// }

// fn dot_access(lhs: Expression, _rhs: String, ctx: Context) -> (usize, Context) {
//     let (_, ctx) = expression(lhs, ctx);
//     ctx.any()
// }

// fn function_call(x: Expression, args: Vec<Expression>, ctx: Context) -> (usize, Context) {
//     let (_, ctx) = expression(x, ctx);
//     let ctx = args.into_iter().fold(ctx, |ctx, x| expression(x, ctx).1);
//     ctx.any()
// }

// fn style(xs: Vec<(String, Expression)>, ctx: Context) -> (usize, Context) {
//     let ctx = xs.into_iter().fold(ctx, |ctx, (_, x)| expression(x, ctx).1);
//     ctx.add_strong(Type::Style)
// }

// fn ksx(x: KSX, ctx: Context) -> (usize, Context) {
//     let ctx = match x {
//         KSX::Text(_) => ctx.add_strong(Type::String).1,
//         KSX::Inline(x) => Context::refer(expression(x, ctx)).1,
//         KSX::Fragment(children) => children.into_iter().fold(ctx, |ctx, x| ksx(x, ctx).1),
//         KSX::Element(name, attributes, children) => {
//             let (_, ctx) = ctx.resolve(&name);
//             let ctx = attributes.into_iter().fold(ctx, |ctx, (name, x)| match x {
//                 Some(x) => expression(x, ctx).1,
//                 None => ctx.resolve(&name).1,
//             });
//             children.into_iter().fold(ctx, |ctx, x| ksx(x, ctx).1)
//         }
//     };

//     ctx.add_strong(Type::Element)
// }

// fn expression(x: Expression, ctx: Context) -> (usize, Context) {
//     match x {
//         Expression::Primitive(x) => primitive(x, ctx),
//         Expression::Identifier(name) => identifier(name, ctx),
//         Expression::Group(x) => group(*x, ctx),
//         Expression::Closure(xs) => closure(xs, ctx),
//         Expression::UnaryOperation(op, x) => unary_operation(op, *x, ctx),
//         Expression::BinaryOperation(op, lhs, rhs) => binary_operation(op, *lhs, *rhs, ctx),
//         Expression::DotAccess(lhs, rhs) => dot_access(*lhs, rhs, ctx),
//         Expression::FunctionCall(x, args) => function_call(*x, args, ctx),
//         Expression::Style(xs) => style(xs, ctx),
//         Expression::KSX(x) => ksx(*x, ctx),
//     }
// }

// fn effect(x: Expression, ctx: Context) -> (usize, Context) {
//     Context::refer(expression(x, ctx))
// }

// fn variable<'a>(name: String, x: Expression, ctx: Context<'a>) -> (usize, Context<'a>) {
//     let (_, ctx) = Context::bind(&name, expression(x, ctx));
//     ctx.add_strong(Type::Nil)
// }

// fn statement<'a>(x: Statement<Expression>, ctx: Context<'a>) -> (usize, Context<'a>) {
//     match x {
//         Statement::Effect(x) => effect(x, ctx),
//         Statement::Variable(name, x) => variable(name, x, ctx),
//     }
// }

// fn type_(value: TypeExpression, ctx: Context) -> (usize, Context) {
//     type_expression(value, ctx)
// }

// fn constant(
//     value_type: Option<TypeExpression>,
//     value: Expression,
//     ctx: Context,
// ) -> (usize, Context) {
//     let ctx = match value_type {
//         Some(x) => type_expression(x, ctx).1,
//         None => ctx,
//     };

//     expression(value, ctx)
// }

// fn enumerated(variants: Vec<(String, Vec<TypeExpression>)>, ctx: Context) -> (usize, Context) {
//     let (variants, ctx) =
//         variants
//             .into_iter()
//             .fold((vec![], ctx), |(mut acc, ctx), (name, params)| {
//                 let (arg_ids, ctx) = params.into_iter().fold((vec![], ctx), |(mut acc, ctx), x| {
//                     let (id, ctx) = type_expression(x, ctx);
//                     acc.push(id);
//                     (acc, ctx)
//                 });
//                 acc.push((name, arg_ids));
//                 (acc, ctx)
//             });

//     ctx.add_strong(Type::Enumerated(variants))
// }

// fn parameter<T>(
//     Parameter {
//         name,
//         value_type,
//         default_value,
//     }: Parameter<T>,
//     ctx: Context,
// ) -> (usize, Context) {
//     let (type_id, ctx) = match value_type {
//         Some(x) => {
//             let (id, ctx) = type_expression(x, ctx);
//             (Some(id), ctx)
//         }
//         None => (None, ctx),
//     };
//     let (default_id, ctx) = match default_value {
//         Some(x) => {
//             let (id, ctx) = expression(x, ctx);
//             (Some(id), ctx)
//         }
//         None => (None, ctx),
//     };

//     let typ = match (type_id, default_id) {
//         (Some(type_id), _) => Context::refer((type_id, ctx)),
//         (_, Some(type_id)) => Context::refer((type_id, ctx)),
//         _ => ctx.any(),
//     };

//     Context::bind(&name, typ)
// }

// fn function<T>(
//     parameters: Vec<Parameter<T>>,
//     body_type: Option<TypeExpression>,
//     body: Expression,
//     ctx: Context,
// ) -> (usize, Context) {
//     let child_ctx = ctx.child();
//     let (param_ids, child_ctx) =
//         parameters
//             .into_iter()
//             .fold((vec![], child_ctx), |(mut acc, ctx), x| {
//                 let (id, ctx) = parameter(x, ctx);
//                 acc.push(id);
//                 (acc, ctx)
//             });
//     let child_ctx = match body_type {
//         Some(x) => type_expression(x, child_ctx).1,
//         None => child_ctx,
//     };
//     let (body_id, _) = expression(body, child_ctx);

//     ctx.add_strong(Type::Function(param_ids, body_id))
// }

// fn view<T>(parameters: Vec<Parameter<T>>, body: Expression, ctx: Context) -> (usize, Context) {
//     let child_ctx = ctx.child();
//     let (param_ids, child_ctx) =
//         parameters
//             .into_iter()
//             .fold((vec![], child_ctx), |(mut acc, ctx), x| {
//                 let (id, ctx) = parameter(x, ctx);
//                 acc.push(id);
//                 (acc, ctx)
//             });
//     let (body_id, _) = expression(body, child_ctx);

//     ctx.add_strong(Type::View(param_ids, body_id))
// }

// fn module<T, U>(
//     imports: Vec<Import>,
//     declarations: Vec<Declaration<T, U>>,
//     ctx: Context,
// ) -> (usize, Context) {
//     let child_ctx = ctx.child();
//     let child_ctx = imports.into_iter().fold(child_ctx, |ctx, x| import(x, ctx));
//     let (declaration_ids, _) =
//         declarations
//             .into_iter()
//             .fold((vec![], child_ctx), |(mut acc, ctx), x| {
//                 let (name, id, ctx) = declaration(x, ctx);
//                 acc.push((name, id));
//                 (acc, ctx)
//             });

//     ctx.add_strong(Type::Module(declaration_ids))
// }

// fn declaration<'a, T, U>(x: Declaration<T, U>, ctx: Context) -> (String, usize, Context) {
//     let bind = |name: String, x| {
//         let (id, ctx) = Context::bind(&name, x);
//         (name, id, ctx)
//     };

//     match x {
//         Declaration::Type {
//             name: Storage(_, name),
//             value,
//         } => bind(name, type_(value, ctx)),
//         Declaration::Constant {
//             name: Storage(_, name),
//             value_type,
//             value,
//         } => bind(name, constant(value_type, value, ctx)),
//         Declaration::Enumerated {
//             name: Storage(_, name),
//             variants,
//         } => bind(name, enumerated(variants, ctx)),
//         Declaration::Function {
//             name: Storage(_, name),
//             parameters,
//             body_type,
//             body,
//         } => bind(name, function(parameters, body_type, body, ctx)),
//         Declaration::View {
//             name: Storage(_, name),
//             parameters,
//             body,
//         } => bind(name, view(parameters, body, ctx)),
//         Declaration::Module {
//             name: Storage(_, name),
//             value: Module {
//                 imports,
//                 declarations,
//             },
//         } => bind(name, module(imports, declarations, ctx)),
//     }
// }

// fn import(Import { path, aliases, .. }: Import, ctx: Context) -> Context {
//     let name = path.last();
//     match (name, aliases) {
//         (Some(name), Some(aliases)) => {
//             aliases
//                 .into_iter()
//                 .fold(ctx, |ctx, (target, alias)| match (target, alias) {
//                     (Target::Module, None) => Context::bind(name, ctx.any()).1,
//                     (_, Some(alias)) => Context::bind(&alias, ctx.any()).1,
//                     (Target::Named(name), None) => Context::bind(&name, ctx.any()).1,
//                 })
//         }
//         (Some(name), None) => Context::bind(name, ctx.any()).1,
//         _ => ctx,
//     }
// }

// fn type_expression(x: TypeExpression, ctx: Context) -> (usize, Context) {
//     match x {
//         TypeExpression::Nil => ctx.add_strong(Type::Nil),
//         TypeExpression::Boolean => ctx.add_strong(Type::Boolean),
//         TypeExpression::Integer => ctx.add_strong(Type::Integer),
//         TypeExpression::Float => ctx.add_strong(Type::Float),
//         TypeExpression::String => ctx.add_strong(Type::String),
//         TypeExpression::Style => ctx.add_strong(Type::Style),
//         TypeExpression::Element => ctx.add_strong(Type::Element),
//         TypeExpression::Identifier(name) => ctx.resolve(&name),
//         TypeExpression::Group(x) => Context::refer(type_expression(*x, ctx)),
//         TypeExpression::DotAccess(lhs, _) => {
//             let (_, ctx) = type_expression(*lhs, ctx);
//             ctx.any()
//         }
//         TypeExpression::Function(params, x) => {
//             let (param_ids, ctx) = params.into_iter().fold((vec![], ctx), |(mut acc, ctx), x| {
//                 let (id, ctx) = type_expression(x, ctx);
//                 acc.push(id);
//                 (acc, ctx)
//             });
//             let (body_id, ctx) = type_expression(*x, ctx);

//             ctx.add_strong(Type::Function(param_ids, body_id))
//         }
//     }
// }

// // pub fn analyze(x: Module) {
// //     let weak_types = RefCell::new(BTreeMap::new());
// //     // let ctx = Context::new(&types);
// //     module(x.imports, x.declarations, Context::new(&weak_types));

// //     let mut strong_types = HashMap::new();
// //     weak_types.borrow().iter().for_each(|(key, value)| {
// //         match value {
// //             WeakType::Strong(typ) => {
// //                 strong_types.insert(key, typ);
// //                 weak_types.borrow_mut().remove(key);
// //             }
// //             WeakType::Reference(id) if strong_types.contains_key(id) => {
// //                 strong_types.insert(key, typ);
// //                 weak_types.borrow_mut().remove(key);
// //             }
// //             _ => todo!(),
// //         }

// //         ()
// //     });
// // }
