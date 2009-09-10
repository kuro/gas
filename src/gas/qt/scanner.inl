/*
 * Copyright 2009 Blanton Black
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file scanner.inl
 * @brief scanner inline implementation
 */

/**
 * @name QDataStream based access.
 */
//@{

template <typename T>
inline
T Scanner::dataValue (const QString& key) const
{
    QDataStream ds (attributes().value(key.toUtf8()));

    T retval;
    ds >> retval;
    return retval;
}

//@}

/**
 * @name QTextStream based access.
 */
//@{

template <typename T>
inline
T Scanner::textValue (const QString& key) const
{
    QTextStream ts (attributes().value(key.toUtf8()));

    T retval;
    ts >> retval;
    return retval;
}

//@}
