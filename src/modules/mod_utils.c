/*
 * This file is part of donky.
 *
 * donky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * donky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @brief Convenient way for modules to check if they were passed args,
 *        and to specify default args to use if not.
 * 
 * @param args Module args.
 * @param otherwise Default args in the event of no module args.
 * 
 * @return One or the other.
 */
char *handle_args(char *args, char *otherwise)
{
        if (args)
                return args;
        else
                return otherwise;
}
