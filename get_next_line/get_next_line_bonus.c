/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_bonus.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tkashi <marvin@42lausanne.ch>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/22 11:49:29 by tkashi            #+#    #+#             */
/*   Updated: 2023/11/22 11:49:34 by tkashi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "get_next_line_bonus.h"
#include <stdio.h>

void	polish(t_list **list)
{
	int		i;
	int		j;
	char	*str;

	free_list(list, FALSE);
	i = 0;
	str = (*list)->tail->buf;
	while (str[i] != '\n' && str[i] != '\0')
		i++;
	if (str[i] == '\0')
	{
		free_list(list, TRUE);
		return ;
	}
	i++;
	j = 0;
	while (str[i] != '\0')
		str[j++] = str[i++];
	str[j] = '\0';
	i = 0;
	while (str[i] != '\n' && str[i] != '\0')
		i++;
	if (str[i] == '\n')
		i++;
	(*list)->len = i;
}

char	*copy_new_line(t_list *list)
{
	int			i;
	int			j;
	t_lst_node	*temp;
	char		*new_line;

	if (!list || list->len == 0)
		return (NULL);
	new_line = (char *)malloc((list->len + 1) * sizeof(char));
	if (!new_line)
		return (NULL);
	temp = list->head;
	j = 0;
	while (j < list->len)
	{
		i = 0;
		while (temp->buf[i] != '\0' && temp->buf[i] != '\n')
			new_line[j++] = temp->buf[i++];
		if (temp->buf[i] == '\n')
			new_line[j++] = '\n';
		temp = temp->next;
	}
	new_line[j] = '\0';
	return (new_line);
}

int	append_to_list(t_list *list, ssize_t bytes_read, char *buf)
{
	ssize_t			new_line_idx;
	t_lst_node		*new_node;

	new_node = create_node(&buf);
	if (!new_node)
		return (0);
	if (!list->head)
		list->head = new_node;
	else
		list->tail->next = new_node;
	list->tail = new_node;
	new_line_idx = find_new_line(list);
	if (new_line_idx == -1)
		list->len += bytes_read;
	else
		list->len += new_line_idx + 1;
	return (1);
}

int	prepare_next_line(int fd, t_list *list)
{
	ssize_t	bytes_read;
	char	*buf;

	while (find_new_line(list) == -1)
	{
		buf = (char *)malloc((BUFFER_SIZE + 1) * sizeof(char));
		if (!buf)
			return (0);
		bytes_read = read(fd, buf, BUFFER_SIZE);
		if (bytes_read <= 0)
		{
			free(buf);
			if (bytes_read < 0)
				return (0);
			return (1);
		}
		buf[bytes_read] = '\0';
		if (!append_to_list(list, bytes_read, buf))
			return (0);
	}
	return (1);
}

/**
 * @brief Reads a line from a file descriptor.
 *
 * This function reads a line from the specified file descriptor and returns it as a string.
 * The function uses a static variable to keep track of the current position in the file,
 * so it can be called multiple times to read successive lines from the same file.
 * i use a maximum of 2048 file descriptors, because the maximum number of open files is 1024 per process.(that will differ on different systems)
 *
 * @param fd The file descriptor to read from.
 * @return A pointer to the line read from the file, or NULL if the end of the file has been reached or an error occurred.
 */
char	*get_next_line(int fd)
{
	char				*next_line;
	static t_list		*list[2048];

	if (BUFFER_SIZE <= 0 || fd >= 2048 || fd < 0 || read(fd, &next_line, 0) < 0)
	{
		free_list(&list[fd], TRUE);
		return (NULL);
	}
	if (!(list[fd]))
	{
		if (!init_list(&list[fd]))
			return (NULL);
	}
	if (!prepare_next_line(fd, list[fd]))
		return (free_list(&list[fd], TRUE));
	next_line = copy_new_line(list[fd]);
	if (!next_line)
		return (free_list(&list[fd], TRUE));
	polish(&list[fd]);
	return (next_line);
}
