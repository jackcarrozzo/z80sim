(if (< 1 (count-windows))
     (delete-other-windows (selected-window)))
 (catch 'tag
   (while t
     (untabify (point-min) (point-max))
     (if buffer-file-name  ; nil for *scratch* buffer
         (progn
           (write-file buffer-file-name)
           (kill-buffer (current-buffer)))
       (throw 'tag t))))
